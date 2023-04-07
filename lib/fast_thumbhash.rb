# frozen_string_literal: true

require "ffi"
require_relative "fast_thumbhash/version"

module FastThumbhash
  def self.thumbhash_to_rgba(
    thumbhash,
    max_size: nil,
    size: nil,
    fill_mode: :no_fill,
    fill_color: nil,
    homogeneous_transform: nil,
    saturation: 0
  )
    binary_thumbhash_to_rgba(
      Base64.decode64(thumbhash),
      max_size: max_size,
      size: size,
      fill_mode: fill_mode,
      fill_color: fill_color,
      homogeneous_transform: homogeneous_transform,
      saturation: saturation
    )
  end

  def self.binary_thumbhash_to_rgba(
    binary_thumbhash,
    max_size: nil,
    size: nil,
    fill_mode: :no_fill,
    fill_color: nil,
    homogeneous_transform: nil,
    saturation: 0
  )
    !max_size.nil? ^ !size.nil? or
      raise ArgumentError, "Pass either the `max_size` option, or an explicit `size`"

    %i[solid blur no_fill].include?(fill_mode) or
      raise ArgumentError, "Invalid `fill_mode` option"

    fill_color_pointer =
      if fill_mode == :solid
        fill_color or
          raise ArgumentError, "`fill_color` is required if fill_mode = :solid"

        fill_color.length == 4 or
          raise ArgumentError, "You need to pass [r, g, b, a] to the `fill_color` option"

        FFI::MemoryPointer.new(:uint8, 4).tap do |p|
          p.write_array_of_uint8(fill_color)
        end
      end

    transform_pointer =
      if homogeneous_transform
        (homogeneous_transform.size == 3 && homogeneous_transform.all? { |row| row.size == 3 }) or
          raise ArgumentError, "`homogeneous_transform` option must be a 3x3 matrix"

        FFI::MemoryPointer.new(:double, 6).tap do |p|
          p.write_array_of_double(
            [
              homogeneous_transform[0][0],
              homogeneous_transform[0][1],
              homogeneous_transform[0][2],
              homogeneous_transform[1][0],
              homogeneous_transform[1][1],
              homogeneous_transform[1][2]
            ]
          )
        end
      end

    thumbhash_pointer = FFI::MemoryPointer.new(:uint8, binary_thumbhash.size)
    thumbhash_pointer.put_array_of_uint8(0, binary_thumbhash.unpack("C*"))

    width, height =
      if size
        size.length == 2 or
          raise ArgumentError, "You need to pass [width, height] to the `size` option"

        size.all? { |dimension| dimension < 100 } or
          raise ArgumentError, "Cannot generate images bigger then 100 pixels"

        size
      else
        max_size <= 100 or
          raise ArgumentError, "Cannot generate images bigger then 100 pixels"

        thumb_size_pointer = FFI::MemoryPointer.new(:uint8, 2)
        Library.thumb_size(thumbhash_pointer, max_size, thumb_size_pointer)
        thumb_size_pointer.read_array_of_uint8(2)
      end

    rgba_size = width * height * 4
    rgba_pointer = FFI::MemoryPointer.new(:uint8, rgba_size)
    Library.thumbhash_to_rgba(
      thumbhash_pointer,
      width,
      height,
      fill_mode.to_sym,
      fill_color_pointer,
      transform_pointer,
      saturation,
      rgba_pointer
    )

    [width, height, rgba_pointer.read_array_of_uint8(rgba_size)]
  ensure
    fill_color_pointer&.free
    transform_pointer&.free
    thumbhash_pointer&.free
    thumb_size_pointer&.free
    rgba_pointer&.free
  end

  def self.rgba_to_thumbhash(width, height, rgba)
    Base64.strict_encode64(rgba_to_binary_thumbhash(width, height, rgba))
  end

  def self.rgba_to_binary_thumbhash(width, height, rgba)
    (width <= 100 && height <= 100) or
      raise ArgumentError, "Encoding an image larger than 100x100 is slow with no benefit"

    rgba_pointer = FFI::MemoryPointer.new(:uint8, rgba.size)
    rgba_pointer.put_array_of_uint8(0, rgba)

    thumbhash_pointer = FFI::MemoryPointer.new(:uint8, 25)

    Library.rgba_to_thumbhash(width, height, rgba_pointer, thumbhash_pointer)

    result = thumbhash_pointer.read_array_of_uint8(25)
    result.pop while result.last.zero?

    result.pack("C*")
  ensure
    rgba_pointer&.free
    thumbhash_pointer&.free
  end

  module Library
    extend FFI::Library
    ffi_lib File.join(File.expand_path(__dir__), "fast_thumbhash.#{RbConfig::CONFIG["DLEXT"]}")

    enum :fill_mode, [
      :no_fill, 0,
      :solid,
      :blur
    ]

    attach_function :thumb_size, %i[pointer uint8 pointer], :size_t
    attach_function :thumbhash_to_rgba, %i[pointer uint8 uint8 fill_mode pointer pointer int pointer], :void
    attach_function :rgba_to_thumbhash, %i[uint8 uint8 pointer pointer], :void
  end
end

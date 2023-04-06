# frozen_string_literal: true

require "ffi"
require "base64"
require_relative "fast_thumbhash/version"

module FastThumbhash
  def self.thumbhash_to_rgba(thumbhash, max_size: 32)
    binary_thumbhash_to_rgba(Base64.decode64(thumbhash), max_size: max_size)
  end

  def self.binary_thumbhash_to_rgba(binary_thumbhash, max_size: 32)
    thumbhash_pointer = FFI::MemoryPointer.new(:uint8, binary_thumbhash.size)
    thumbhash_pointer.put_array_of_uint8(0, binary_thumbhash.unpack("C*"))

    thumb_size_pointer = FFI::MemoryPointer.new(:uint8, 2)
    Library.thumb_size(thumbhash_pointer, max_size, thumb_size_pointer)
    width, height = thumb_size_pointer.read_array_of_uint8(2)

    rgba_size = width * height * 4
    rgba_pointer = FFI::MemoryPointer.new(:uint8, rgba_size)
    Library.thumbhash_to_rgba(thumbhash_pointer, width, height, rgba_pointer)

    [width, height, rgba_pointer.read_array_of_uint8(rgba_size)]
  ensure
    thumbhash_pointer&.free
    thumb_size_pointer&.free
    rgba_pointer&.free
  end

  def self.rgba_to_thumbhash(width, height, rgba)
    Base64.strict_encode64(rgba_to_binary_thumbhash(width, height, rgba))
  end

  def self.rgba_to_binary_thumbhash(width, height, rgba)
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
    attach_function :thumb_size, %i[pointer uint8 pointer], :size_t
    attach_function :thumbhash_to_rgba, %i[pointer uint8 uint8 pointer], :void
    attach_function :rgba_to_thumbhash, %i[uint8 uint8 pointer pointer], :void
  end
end
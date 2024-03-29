# frozen_string_literal: true

require "base64"
require "chunky_png"
require "thumbhash"

RSpec.describe FastThumbhash do
  it "has a version number" do
    expect(described_class::VERSION).not_to be nil
  end

  samples = %w[
    IckCLIQNmHiIeYmCe32PjPjaqA==
    wQeCBQAi/OyIh0hiV/yICQAAeXmGiGCoxw==
    mIeBAYQT33p4iGoghwVyWImZlnegmXY=
    LUuGLAQs93doyFiJijIHOI+GcDRYKGh3BA==
    ZRuDAoInAFN5eHf/hvhviL+8S3mZaGg=
    rsYJLJZ4d4iAeIiAh5togIk3+A==
    4igGDIL2iWaPdIVzaXuJj4BwWA==
    EQqCDQIkrHOfVG8wBa2f3tz7CXeqZWFLdg==
    CwgGH4Z3q3cgd2SkW3mHqVh3R8YJZYsA
  ].freeze

  describe ".rgba_to_thumbhash" do
    it "converts an RGBA stream into a base64-encoded thumbhash" do
      samples.each_with_index do |expected_thumbhash, index|
        image = ChunkyPNG::Image.from_file("spec/samples/#{index}.png")
        rgba = image.to_rgba_stream.unpack("C*")

        thumbhash = described_class.rgba_to_thumbhash(image.width, image.height, rgba)

        expect(thumbhash).to eq(expected_thumbhash)
      end
    end
  end

  describe ".thumbhash_to_rgba" do
    it "converts a base-64-encoded thumbhash into an RGBA stream" do
      samples.each_with_index do |thumbhash, index|
        w, h, rgba = described_class.thumbhash_to_rgba(thumbhash, max_size: 32)

        # To regenerate thumbhashes:
        # thumbhash_image = ChunkyPNG::Image.new(w, h, rgba.pack("C*").unpack("N*"))
        # thumbhash_image.save("spec/samples/#{index}_thumbhash.png")

        expected_image = ChunkyPNG::Image.from_file("spec/samples/#{index}_thumbhash.png")
        expect(w).to eq expected_image.width
        expect(h).to eq expected_image.height
        expect(rgba).to eq expected_image.to_rgba_stream.unpack("C*")
      end
    end

    describe "additional options" do
      context "homogeneous_transform" do
        it "crop aligning to right works" do
          w, h, rgba1 = described_class.thumbhash_to_rgba(
            samples.first,
            size: [32, 32],
            homogeneous_transform: [
              [0.5, 0.0, 0.5],
              [0.0, 1.0, 0.0],
              [0.0, 0.0, 1.0]
            ]
          )

          thumb1 = ChunkyPNG::Image.new(w, h, rgba1.pack("C*").unpack("N*"))

          w, h, rgba2 = described_class.thumbhash_to_rgba(
            samples.first,
            size: [64, 32]
          )

          thumb2 = ChunkyPNG::Image.new(w, h, rgba2.pack("C*").unpack("N*"))

          expect(thumb1.get_pixel(31, 0)).to eq(thumb2.get_pixel(63, 0))
          expect(thumb1.get_pixel(0, 0)).to eq(thumb2.get_pixel(32, 0))
        end
      end

      context "explicit size" do
        it "changes size" do
          w, h, = described_class.thumbhash_to_rgba(
            samples.first,
            size: [20, 30]
          )

          expect(w).to eq 20
          expect(h).to eq 30
        end
      end

      context "saturation=-100" do
        it "gray-scales the image" do
          _, _, rgba = described_class.thumbhash_to_rgba(
            samples.first,
            max_size: 32,
            saturation: -100
          )

          r = rgba[0]
          g = rgba[1]
          b = rgba[2]

          expect(r).to eq(g)
          expect(g).to eq(b)
        end
      end

      context "fill_color" do
        it "fill_color solid fills the image with fill_color" do
          w, h, rgba = described_class.thumbhash_to_rgba(
            samples[1],
            max_size: 32,
            fill_mode: :solid,
            fill_color: [255, 0, 0, 255]
          )

          thumb = ChunkyPNG::Image.new(w, h, rgba.pack("C*").unpack("N*"))

          expect(thumb.get_pixel(0, 0)).to eq ChunkyPNG::Color(255, 0, 0, 255)
        end
      end

      context "fill_mode=blur" do
        it "works" do
          w, h, rgba = described_class.thumbhash_to_rgba(
            "m1sGHgRT9tenV3fKRo6Zl7bW9X2KcEc=",
            size: [10, 25],
            fill_mode: :blur,
            fill_color: [255, 0, 0, 68],
            homogeneous_transform: [
              [1.0, 0.0, 0.0],
              [0.0, 2.2, -0.60],
              [0.0, 0.0, 1.0]
            ]
          )

          thumb = ChunkyPNG::Image.new(w, h, rgba.pack("C*").unpack("N*"))
          expect(thumb.get_pixel(0, 0)).not_to eq ChunkyPNG::Color(255, 0, 0, 68)
        end
      end
    end
  end
end

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
  end
end

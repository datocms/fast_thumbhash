# frozen_string_literal: true

require "chunky_png"

RSpec.describe FastThumbhash do
  it "has a version number" do
    expect(described_class::VERSION).not_to be nil
  end

  it "does something useful" do
    image = ChunkyPNG::Image.from_file("image.png")

    thumbhash = described_class.rgba_to_thumbhash(
      image.width,
      image.height,
      image.to_rgba_stream.unpack("C*")
    )

    width, height, data = described_class.thumbhash_to_rgba(thumbhash)
    ChunkyPNG::Image.new(width, height, data.pack("C*").unpack("N*")).save("thumbhash.png")
  end
end

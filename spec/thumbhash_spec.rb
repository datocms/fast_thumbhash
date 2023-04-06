# frozen_string_literal: true

require "chunky_png"

RSpec.describe FastThumbhash do
  it "has a version number" do
    expect(described_class::VERSION).not_to be nil
  end

  describe ".binary_thumbhash_to_rgba" do
    [
      { thumbhash: "1QcSHQRnh493V4dIh4eXh1h4kJUI", w: 4, h: 5, rgba: "TlZ0/3Fzhf9pb4D/VmiB/52Rl//JtrH/t6ii/56bnv9lTkX/lndk/4BlUv9fT0b/EQsO/yodF/8tIR3/Jyct/wAEHf8HEiP/Eh4x/wARL/8=" },
      { thumbhash: "3PcNNYSFeXh/d3eld0iHZoZgVwh2", w: 5, h: 4, rgba: "V3i6/1h2tf9ngLf/e422/3yJqP99jqv/eYml/5Ccs/+5v8z/tba7/05NO/86OSn/UU49/3x1YP+Bdlv/YVUo/0Y7Ef9YTCX/XU4l/1lIG/8=" },
      { thumbhash: "3OcRJYB4d3h/iIeHeEh3eIhw+j3A", w: 5, h: 4, rgba: "j5uz/5Ofuf+Ll7T/ipax/5CctP+cqLv/oKzD/5aivP+cqMH/k5+2/y05K/9BTUT/RFBL/ztHQv8wPDb/TFgg/1RgLf9PWy3/LDgL/z1KG/8=" },
      { thumbhash: "HBkSHYSIeHiPiHh8eJd4eTN0EEQG", w: 5, h: 4, rgba: "w+n2/77Tzf/Mzbj/1NC//9DOyf94e2b/bV83/4dnMf+LZjP/gmE6/2BZPP9oUCH/fVUX/3dNE/9gOwz/P01I/1hWP/9WRiD/ZlUy/2JWP/8=" },
      { thumbhash: "VggKDYAW6lZvdYd6d2iZh/p4GE/k", w: 5, h: 4, rgba: "bWlT/2BcU//MyMf/e3dx/3l1ZP9UUDv/Qz83/7Csrf9oZF//X1tL/1JOOP8fGxL/eHR1/zMvKv85NSf/TEgy/yEdE/+JhYX/OjYy/0VBMv8=" },
      { thumbhash: "2fcZFIB3iId/h3iJh4aIYJ2V8g", w: 5, h: 3, rgba: "s7fE/6mtu//S1uT/0tbh/7O3wP89QU//QUVW/0hMXv9QVGb/VVlq/yAkB/8iJg3/HCAL/yUpFv8TFwT/" },
      { thumbhash: "IQgSLYZ6iHePh4h1eFeHh4dwgwg3", w: 5, h: 4, rgba: "mbzu/4Gm2f+NtOT/dZrF/2+SuP/k3Nv/yMPD/9/c2/+2sq3/oJqQ/559YP+CZUn/jnRX/3ZbPP9xVjL/Z087/2BLOf9TQjD/RTUg/0g3H/8=" },
      { thumbhash: "YJqGPQw7sFltlqhFafSE+Q6oJ1h2iHB2Rw", w: 5, h: 5, rgba: "4FVDAKFocgB1howAorV4AMe2MQDjWU0Gomp4ZHaIkV+uwIZ74M9MAN9COAOaTl9caWVvSqOfZWngtjQA/TEkAbs/THaGUFZyuYFChPSWDgv/JhUA4ztDALBNTQDeeDMA/4kAAA==" },
      { thumbhash: "2IqDBQQnxnj0JoLYdM3P8ahpuDeHiHdwZw", w: 5, h: 5, rgba: "8A8jAOsUIxnkHCMA3SIjFtomIwDxDyM57BMjJ+UbIwDfISMU2yQjI+8QI0DrFSMC5BsjAN8hIwDcJCM16xUjIecZIyLhHyMA3CMjFtomIyvmGSMA4x0jMN0iIwDZJiMg1ykjAA==" },
    ].each do |example|
      it "works as thumbhash with `#{example[:thumbhash]}`" do
        w, h, rgba = described_class.thumbhash_to_rgba(example[:thumbhash], max_size: 5)
        expect(w).to eq example[:w]
        expect(h).to eq example[:h]
        expect(Base64.strict_encode64(rgba.pack("C*"))).to eq example[:rgba]
      end
    end
  end

  describe ".rgba_to_binary_thumbhash" do
    it "works as thumbhash" do
      image = ChunkyPNG::Image.from_file("image.png")
      rgba = image.to_rgba_stream.unpack("C*")

      thumbhash = described_class.rgba_to_thumbhash(image.width, image.height, rgba)
      expect(thumbhash).to eq "KToGFoLqV5hvd5aah1iIuVZ3+I6Mz7g="

      width, height, data = described_class.thumbhash_to_rgba(thumbhash, max_size: 32)
      ChunkyPNG::Image.new(width, height, data.pack("C*").unpack("N*")).save("thumbhash.png")
    end
  end
end

# FastThumbhash

FastThumbhash is a Ruby gem that provides a highly optimized implementation of the [ThumbHash algorithm](https://evanw.github.io/thumbhash/), a compact representation of an image placeholder for a smoother loading experience.

To achieve these benefits, FastThumbhash is implemented as a Ruby C extension, which means that the core functionality of the ThumbHash algorithm is written in C for faster execution times compared to a pure Ruby implementation. This makes FastThumbhash ideal for applications or websites that require high-performance image processing.

## Installation

Install the gem and add to the application's Gemfile by executing:

    $ bundle add fast_thumbhash

If bundler is not being used to manage dependencies, install the gem by executing:

    $ gem install fast_thumbhash

## Usage

Using the ChunkyPNG library to process the image, this example shows how to generate a thumbhash that can be stored alongside an image as well as the method to convert the hash into a data string for the image placeholder.

```ruby
require "fast_thumbhash"
require "chunky_png"
require "base64"

# load an image
image = ChunkyPNG::Image.from_file("image.png")

# convert the image into a base64-encoded thumbhash
thumbhash = FastThumbhash.rgba_to_thumbhash(image.width, image.height, image.to_rgba_stream.unpack("C*"))

puts thumbhash # => "rsYJLJZ4d4iAeIiAh5togIk3+A=="

# convert the thumbhash back to an RGBA stream
width, height, rgba = FastThumbhash.thumbhash_to_rgba(thumbhash, max_size: 32)

# generate a placeholder image based on thumbhash
placeholder = ChunkyPNG::Image.new(width, height, rgba.pack("C*").unpack("N*"))

# save placeholder to file
options = { compression: Zlib::BEST_COMPRESSION, filtering: ChunkyPNG::FILTER_PAETH, interlace: false }
placeholder.save("placeholder.png", options)

# generate a DataURL string for the pleaceholder
thumbhash_image_blob = "data:image/png;base64,#{Base64.strict_encode64(thumbhash_image.to_blob(options))}"
```

## Development

After checking out the repo, run `bin/setup` to install dependencies. Then, run `rake spec` to run the tests. You can also run `bin/console` for an interactive prompt that will allow you to experiment.

To install this gem onto your local machine, run `bundle exec rake install`. To release a new version, update the version number in `version.rb`, and then run `bundle exec rake release`, which will create a git tag for the version, push git commits and the created tag, and push the `.gem` file to [rubygems.org](https://rubygems.org).

## Releasing a new version

First update the FastThumbhash::VERSION, then run `rake release`.

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/stefanoverna/fast_thumbhash. This project is intended to be a safe, welcoming space for collaboration, and contributors are expected to adhere to the [code of conduct](https://github.com/stefanoverna/fast_thumbhash/blob/master/CODE_OF_CONDUCT.md).

## License

The gem is available as open source under the terms of the [MIT License](https://opensource.org/licenses/MIT).

## Code of Conduct

Everyone interacting in the FastThumbhash project's codebases, issue trackers, chat rooms and mailing lists is expected to follow the [code of conduct](https://github.com/stefanoverna/fast_thumbhash/blob/master/CODE_OF_CONDUCT.md).

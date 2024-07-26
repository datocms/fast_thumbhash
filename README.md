<!--datocms-autoinclude-header start-->

<a href="https://www.datocms.com/"><img src="https://www.datocms.com/images/full_logo.svg" height="60"></a>

👉 [Visit the DatoCMS homepage](https://www.datocms.com) or see [What is DatoCMS?](#what-is-datocms)

---

<!--datocms-autoinclude-header end-->

# FastThumbhash

[![Ruby](https://github.com/datocms/fast_thumbhash/actions/workflows/main.yml/badge.svg)](https://github.com/datocms/fast_thumbhash/actions/workflows/main.yml)

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

### Additional Options

This section covers additional options available for the `.thumbhash_to_rgba` method.

##### `max_size`

The max_size option allows you to request a thumbnail up to the specified size. This is the suggested option for most use cases.

```ruby
w, h, rgba = described_class.thumbhash_to_rgba(thumbhash, max_size: 32)

puts [w, h].inspect # => final image is [10, 32]
```

##### `homogeneous_transform` and `size`

The `homogeneous_transform` option allows you to apply a homogeneous transformation matrix when creating a thumbnail. The transformation matrix is a 3x3 array. This can be useful for applying roto-transformations to the thumbnail. The `size` option is useful to explicitly specify the width/height of the thumbnail when combined with the `homogeneous_transform` option to create a thumbnail with specific dimensions and transformations applied to it.


```ruby
w, h, rgba = described_class.thumbhash_to_rgba(
  thumbhash,
  size: [32, 32],
  homogeneous_transform: [
    [0.5, 0.0, 0.5],
    [0.0, 1.0, 0.0],
    [0.0, 0.0, 1.0]
  ]
)
```

##### `saturation`

The `saturation` option adjusts the saturation of the thumbnail's colors. It accepts a value in the range of -100 to +100. A value of -100 will result in a grayscale image.

```ruby
w, h, rgba = described_class.thumbhash_to_rgba(
  thumbhash,
  max_size: 32,
  saturation: -100
)
```

##### `fill_mode`

The `fill_mode` option specifies how to fill in any transparent areas in your image with a color of your choice. When `fill_mode` is `:solid`, you need to pass an additional `fill_color` option to specify the actual RGBA color to use. When `fill_mode` is `:blur`, the excess space will be filled with a blurred version of the original image itself. When `fill_image` is `:clamp`, the excess space is filled with extended pixels from the edge of the image.

```ruby
w, h, rgba = described_class.thumbhash_to_rgba(
  thumbhash,
  max_size: 32,
  fill_mode: :solid,
  fill_color: [255, 0, 0, 100]
)
```

## Development

After checking out the repo, run `bin/setup` to install dependencies. Then, run `rake spec` to run the tests. You can also run `bin/console` for an interactive prompt that will allow you to experiment.

To install this gem onto your local machine, run `bundle exec rake install`. To release a new version, update the version number in `version.rb`, and then run `bundle exec rake release`, which will create a git tag for the version, push git commits and the created tag, and push the `.gem` file to [rubygems.org](https://rubygems.org).

## Releasing a new version

First update the FastThumbhash::VERSION, then run `rake release`.

## Contributing

Bug reports and pull requests are welcome on GitHub at https://github.com/datocms/fast_thumbhash. This project is intended to be a safe, welcoming space for collaboration, and contributors are expected to adhere to the [code of conduct](https://github.com/datocms/fast_thumbhash/blob/master/CODE_OF_CONDUCT.md).

## License

The gem is available as open source under the terms of the [MIT License](https://opensource.org/licenses/MIT).

## Code of Conduct

Everyone interacting in the FastThumbhash project's codebases, issue trackers, chat rooms and mailing lists is expected to follow the [code of conduct](https://github.com/datocms/fast_thumbhash/blob/master/CODE_OF_CONDUCT.md).

<!--datocms-autoinclude-footer start-->

---

# What is DatoCMS?
<a href="https://www.datocms.com/"><img src="https://www.datocms.com/images/full_logo.svg" height="60"></a>

[DatoCMS](https://www.datocms.com/) is the REST & GraphQL Headless CMS for the modern web.

Trusted by over 25,000 enterprise businesses, agency partners, and individuals across the world, DatoCMS users create online content at scale from a central hub and distribute it via API. We ❤️ our [developers](https://www.datocms.com/team/best-cms-for-developers), [content editors](https://www.datocms.com/team/content-creators) and [marketers](https://www.datocms.com/team/cms-digital-marketing)!

**Quick links:**

- ⚡️ Get started with a [free DatoCMS account](https://dashboard.datocms.com/signup)
- 🔖 Go through the [docs](https://www.datocms.com/docs)
- ⚙️ Get [support from us and the community](https://community.datocms.com/)
- 🆕 Stay up to date on new features and fixes on the [changelog](https://www.datocms.com/product-updates)

**Our featured repos:**
- [datocms/react-datocms](https://github.com/datocms/react-datocms): React helper components for images, Structured Text rendering, and more
- [datocms/js-rest-api-clients](https://github.com/datocms/js-rest-api-clients): Node and browser JavaScript clients for updating and administering your content. For frontend fetches, we recommend using our [GraphQL Content Delivery API](https://www.datocms.com/docs/content-delivery-api) instead.
- [datocms/cli](https://github.com/datocms/cli): Command-line interface that includes our [Contentful importer](https://github.com/datocms/cli/tree/main/packages/cli-plugin-contentful) and [Wordpress importer](https://github.com/datocms/cli/tree/main/packages/cli-plugin-wordpress)
- [datocms/plugins](https://github.com/datocms/plugins): Example plugins we've made that extend the editor/admin dashboard
- [DatoCMS Starters](https://www.datocms.com/marketplace/starters) has examples for various Javascript frontend frameworks

Or see [all our public repos](https://github.com/orgs/datocms/repositories?q=&type=public&language=&sort=stargazers)

<!--datocms-autoinclude-footer end-->

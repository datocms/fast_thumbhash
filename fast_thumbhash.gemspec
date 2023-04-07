# frozen_string_literal: true

require_relative "lib/fast_thumbhash/version"

Gem::Specification.new do |spec|
  spec.name = "fast_thumbhash"
  spec.version = FastThumbhash::VERSION
  spec.authors = ["Stefano Verna"]
  spec.email = ["s.verna@datocms.com"]

  spec.summary = "Ruby C extension to encode/decode ThumbHash"
  spec.description = "Provides a highly optimized implementation of the ThumbHash algorithm, a compact representation of an image placeholder for a smoother loading experience"
  spec.homepage = "https://github.com/datocms/fast_thumbhash"
  spec.license = "MIT"
  spec.required_ruby_version = ">= 2.6.0"

  spec.metadata["homepage_uri"] = spec.homepage
  spec.metadata["source_code_uri"] = spec.homepage
  spec.metadata["rubygems_mfa_required"] = "true"

  # Specify which files should be added to the gem when it is released.
  # The `git ls-files -z` loads the files in the RubyGem that have been added into git.
  spec.files = Dir.chdir(__dir__) do
    `git ls-files -z`.split("\x0").reject do |f|
      (f == __FILE__) || f.match(%r{\A(?:(?:bin|test|spec|features)/|\.(?:git|travis|circleci)|appveyor)})
    end
  end
  spec.bindir = "exe"
  spec.executables = spec.files.grep(%r{\Aexe/}) { |f| File.basename(f) }
  spec.extensions    = %w[ext/fast_thumbhash/extconf.rb]
  spec.require_paths = ["lib"]

  spec.add_dependency "ffi"
end

# frozen_string_literal: true

require "ffi"
require_relative "fast_thumbhash/version"

module FastThumbhash
  def self.myfunc
    Library.myfunc
  end

  module Library
    extend FFI::Library
    ffi_lib File.join(File.expand_path(__dir__), "fast_thumbhash.#{RbConfig::CONFIG["DLEXT"]}")
    attach_function :myfunc, [], :void
  end
end

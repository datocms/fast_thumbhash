# frozen_string_literal: true

require "bundler/gem_tasks"
require "rspec/core/rake_task"
require "rake/extensiontask"
require "rubocop/rake_task"

RSpec::Core::RakeTask.new(:spec)
RuboCop::RakeTask.new

Rake::Task[:build].enhance %i[spec rubocop]
Rake::Task[:spec].enhance %i[compile:fast_thumbhash]

Rake::ExtensionTask.new "fast_thumbhash" do |ext|
  ext.lib_dir = "lib"
end

task default: %i[spec rubocop]

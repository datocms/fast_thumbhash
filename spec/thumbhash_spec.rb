# frozen_string_literal: true

RSpec.describe FastThumbhash do
  it "has a version number" do
    expect(described_class::VERSION).not_to be nil
  end

  it "does something useful" do
    described_class.myfunc
  end
end

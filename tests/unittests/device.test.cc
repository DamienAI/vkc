#include <catch2/catch.hpp>

#include <vk/vk.hpp>

SCENARIO("API should provide API to find and use Vulkan devices", "[Vk::api::Device]") {
  GIVEN("A computer with a Vulkan enabled device") {
    THEN("the device should be usable") {
      REQUIRE_NOTHROW(Vk::api::Device::findFirstAvailable(true));
    }
  }
}
#include <catch2/catch.hpp>

#include <stdexcept>

#include <vk/vk.hpp>

SCENARIO("Vk::ArrayBuffer should be used to transfer back and forth data to the gpu", "[Vk::ArrayBuffer]") {
  GIVEN("A GPU device interface") {

    auto device = Vk::api::Device::findFirstAvailable(true);

    WHEN("the specific size is provided") {

      auto buffer = Vk::ArrayBuffer<float>(device, 16);

      THEN("the elements count must match the allocated count") {
        REQUIRE(buffer.getElementsCount() == 16UL);
      }
    }

    WHEN("the input data is in a vector") {

      auto data = std::vector<float>{1.f, 2.f, 3.f, 4.f};
      auto buffer = Vk::ArrayBuffer<float>(device, data);

      THEN("the buffer size must match the input buffer size") {
        REQUIRE(buffer.getElementsCount() == 4UL);
      }
    }

    WHEN("the data is uploaded") {

      auto data = std::vector<float>{1.f, 2.f, 3.f, 4.f};
      auto buffer = Vk::ArrayBuffer<float>(device, 4);

      THEN("the upload should not throw") {
        REQUIRE_NOTHROW(buffer.fromVector(data));
      }
    }

    WHEN("the data is uploaded in a buffer not large enough") {

      auto data = std::vector<float>{1.f, 2.f, 3.f, 4.f};
      auto buffer = Vk::ArrayBuffer<float>(device, 2);

      THEN("the upload should throw") {
        REQUIRE_THROWS_AS(buffer.fromVector(data), std::runtime_error);
      }
    }

    WHEN("the data is downloaded on the GPU then copied back to CPU") {
      auto data = std::vector<float>{1.f, 2.f, 3.f, 4.f};
      auto buffer = Vk::ArrayBuffer<float>(device, data);
      auto outputVector = buffer.toVector();

      THEN("the copy back to the CPU should provide same buffer size") {
        REQUIRE(outputVector.size() == data.size());
      }

      THEN("the data must be the same") {
        REQUIRE(data == outputVector);
      }
    }
  }
}

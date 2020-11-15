#include <catch2/catch.hpp>
#include <stdexcept>

#include <vk/vk.hpp>

SCENARIO("It should be possible to run compute shaders on the gpu using vulkan", "[Vk::ComputeProgram]") {
  auto device = Vk::api::Device::findFirstAvailable(true);
  GIVEN("a GPU device") {
    THEN("it should be possible to create a program") {
      REQUIRE_NOTHROW(Vk::ComputeProgram(device, "tests/unittests/fixtures/shaders/grids.comp.spv"));
    }
  }
  GIVEN("a compute program") {
    auto program = Vk::ComputeProgram(device, "tests/unittests/fixtures/shaders/grids.comp.spv");
    THEN("it should provide an api to setup workgroups") {
      auto count = Vk::WorkGroupsCount{1, 2, 3};
      program.withWorkGroups(count);
      REQUIRE(program.getWorkGroups() == count);
    }
  }
  GIVEN("a specialization") {
    using Specs = Vk::typelist<uint32_t, uint32_t, uint32_t>;
    THEN("it should be possible to have compute program which use this it") {
      auto program = Vk::ComputeProgram<Specs>(device, "tests/unittests/fixtures/shaders/threadscount.comp.spv");

      auto count = Vk::WorkGroupsCount{2, 2, 1};
      auto size = Vk::WorkGroupSize{4, 4, 1};

      auto threadsCount =
        count[0] * size[0] *
        count[1] * size[1] *
        count[2] * size[2];

      auto output = Vk::ArrayBuffer<uint32_t>(device, 1);

      program
        .withSpecializations(size[0], size[1], size[2])
        .withWorkGroups(count)
        (output);

      auto outputVec = output.toVector();
      REQUIRE(outputVec[0] == threadsCount);
    }
  }
  GIVEN("a 3D workload") {
    THEN("should be possible to run a 3d computer kernel") {
      using Specs = Vk::typelist<uint32_t, uint32_t, uint32_t>;

      auto program = Vk::ComputeProgram<Specs>(device, "tests/unittests/fixtures/shaders/threadscount.comp.spv");

      auto count = Vk::WorkGroupsCount{2, 2, 2};
      auto size = Vk::WorkGroupSize{4, 4, 2};

      auto threadsCount =
        count[0] * size[0] *
        count[1] * size[1] *
        count[2] * size[2];

      auto output = Vk::ArrayBuffer<uint32_t>(device, 1);

      program
        .withSpecializations(size[0], size[1], size[2])
        .withWorkGroups(count)
        (output);

      auto outputVec = output.toVector();
      REQUIRE(outputVec[0] == threadsCount);
    }
  }
  GIVEN("a number of threads to run") {
    auto elementsCount = 21U;
    THEN("it should be able to run exactly the required threads count") {
      using Specs = Vk::typelist<uint32_t, uint32_t, uint32_t>;

      struct Constants {
        uint32_t elemenstCount;
      };

      auto program = Vk::ComputeProgram<Specs, Constants>(device, "tests/unittests/fixtures/shaders/bounds.comp.spv");

      auto count = Vk::WorkGroupsCount{2, 2, 2};
      auto size = Vk::WorkGroupSize{4, 4, 2};

      auto output = Vk::ArrayBuffer<uint32_t>(device, 1);

      program
        .withSpecializations(size[0], size[1], size[2])
        .withWorkGroups(count)
        ({elementsCount}, output);

      auto outputVec = output.toVector();
      REQUIRE(outputVec[0] == elementsCount);
    }
  }
  GIVEN("specialization structure and a valid program") {
    using Specs = Vk::typelist<float, uint32_t, bool>;
    auto program = Vk::ComputeProgram<Specs>(device, "tests/unittests/fixtures/shaders/specs.comp.spv");
    auto count = Vk::WorkGroupsCount{1, 1, 1};
    auto output = Vk::ArrayBuffer<float>(device, 3);

    THEN("it should be provided to the shaders") {
      program
          .withSpecializations(4.32f, 17, true)
          .withWorkGroups(count)
          (output);

      auto outputVec = output.toVector();
      REQUIRE(outputVec[0] == 4.32f);
      REQUIRE(outputVec[1] == 17.0f);
      REQUIRE(outputVec[2] == 42.0f);
    }

    THEN("it should be updated if using other values") {
      program
    .withSpecializations(8.32f, 15, false)
    .withWorkGroups(count)
    (output);

    auto outputVec = output.toVector();
    REQUIRE(outputVec[0] == 8.32f);
    REQUIRE(outputVec[1] == 15.0f);
    REQUIRE(outputVec[2] == -42.0f);
    }
  }
  GIVEN("a simple program") {
    using Specs = Vk::typelist<uint32_t, uint32_t, uint32_t>;
    auto program = Vk::ComputeProgram(device, "tests/unittests/fixtures/shaders/grids.comp.spv");
    
    THEN("it should have the right parameters") {
      auto count = Vk::WorkGroupsCount{2, 2, 1};
      auto size = Vk::WorkGroupSize{4, 4, 1};

      auto threadsCount = count[0] * size[0] * count[1] * size[1] * count[2] * size[2];
      auto output = Vk::ArrayBuffer<uint32_t>(device, threadsCount * 4 + 1);

      program
        .withWorkGroups(count)
        (output);

      auto outputVec = output.toVector();
      
      // Offset 0 contains result of atomicAdd so should be eq to number of threads
      auto it = outputVec.cbegin();
      auto value = *it++;
      REQUIRE(value == threadsCount);

      while (it != outputVec.cend()) {
        auto entryIdx = (std::distance(outputVec.cbegin(), it) - 1) / 4;
        auto threadIdx = *it++;
        auto values = std::array<uint32_t, 3>{*it++, *it++, *it++};
        REQUIRE(threadIdx == entryIdx);
        const auto contentIdx = threadIdx&3;
        if (contentIdx == 2) {
          REQUIRE(values[0] == count[0]);
          REQUIRE(values[1] == count[1]);
          REQUIRE(values[2] == count[2]);
        }
      }
    }
  }
}

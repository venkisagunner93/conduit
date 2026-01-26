#include "conduit_core/internal/shm_region.hpp"
#include "conduit_core/exceptions.hpp"

#include <gtest/gtest.h>
#include <cstring>

using namespace conduit::internal;
using namespace conduit;

class ShmRegionTest : public ::testing::Test {
protected:
    void TearDown() override {
        // Clean up any test regions
        ShmRegion::unlink("test_region_1");
        ShmRegion::unlink("test_region_2");
        ShmRegion::unlink("test_region_3");
        ShmRegion::unlink("test_region_4");
        ShmRegion::unlink("test_region_nonexistent");
    }
};

TEST_F(ShmRegionTest, test_create_and_open) {
    const std::string name = "test_region_1";
    const size_t size = 4096;

    // Cleanup first
    ShmRegion::unlink(name);

    // Create
    auto region1 = ShmRegion::create(name, size);
    ASSERT_EQ(region1.size(), size);
    ASSERT_NE(region1.data(), nullptr);
    EXPECT_EQ(region1.name(), name);

    // Write data
    std::memset(region1.data(), 0xAB, size);

    // Open from another "process" (same process for test)
    auto region2 = ShmRegion::open(name);
    ASSERT_EQ(region2.size(), size);

    // Verify data visible
    EXPECT_EQ(std::memcmp(region1.data(), region2.data(), size), 0);

    // Verify the data is actually 0xAB
    auto* bytes = static_cast<uint8_t*>(region2.data());
    for (size_t i = 0; i < size; ++i) {
        ASSERT_EQ(bytes[i], 0xAB) << "Mismatch at index " << i;
    }
}

TEST_F(ShmRegionTest, test_exists) {
    const std::string name = "test_region_2";
    ShmRegion::unlink(name);

    EXPECT_FALSE(ShmRegion::exists(name));

    {
        auto region = ShmRegion::create(name, 1024);
        EXPECT_TRUE(ShmRegion::exists(name));
    }

    // Still exists after region goes out of scope (unmap doesn't unlink)
    EXPECT_TRUE(ShmRegion::exists(name));

    ShmRegion::unlink(name);
    EXPECT_FALSE(ShmRegion::exists(name));
}

TEST_F(ShmRegionTest, test_create_already_exists) {
    const std::string name = "test_region_3";
    ShmRegion::unlink(name);

    auto region1 = ShmRegion::create(name, 1024);

    // Second create should fail
    EXPECT_THROW(ShmRegion::create(name, 1024), ShmError);
}

TEST_F(ShmRegionTest, test_open_not_found) {
    const std::string name = "test_region_nonexistent";
    ShmRegion::unlink(name);

    EXPECT_THROW(ShmRegion::open(name), ShmError);
}

TEST_F(ShmRegionTest, test_move_semantics) {
    const std::string name = "test_region_4";
    ShmRegion::unlink(name);

    auto region1 = ShmRegion::create(name, 1024);
    void* ptr = region1.data();
    size_t size = region1.size();

    // Move construct
    ShmRegion region2 = std::move(region1);

    EXPECT_EQ(region2.data(), ptr);
    EXPECT_EQ(region2.size(), size);
    EXPECT_EQ(region1.data(), nullptr);  // moved from
    EXPECT_EQ(region1.size(), 0);
}

TEST_F(ShmRegionTest, test_move_assignment) {
    const std::string name1 = "test_region_1";
    const std::string name2 = "test_region_2";
    ShmRegion::unlink(name1);
    ShmRegion::unlink(name2);

    auto region1 = ShmRegion::create(name1, 1024);
    auto region2 = ShmRegion::create(name2, 2048);

    void* ptr1 = region1.data();

    // Move assign
    region2 = std::move(region1);

    EXPECT_EQ(region2.data(), ptr1);
    EXPECT_EQ(region2.size(), 1024);
    EXPECT_EQ(region2.name(), name1);
    EXPECT_EQ(region1.data(), nullptr);
}

TEST_F(ShmRegionTest, test_zero_initialized) {
    const std::string name = "test_region_1";
    ShmRegion::unlink(name);

    auto region = ShmRegion::create(name, 1024);

    // Verify memory is zero-initialized
    auto* bytes = static_cast<uint8_t*>(region.data());
    for (size_t i = 0; i < 1024; ++i) {
        ASSERT_EQ(bytes[i], 0) << "Non-zero at index " << i;
    }
}

TEST_F(ShmRegionTest, test_shared_writes) {
    const std::string name = "test_region_1";
    ShmRegion::unlink(name);

    auto region1 = ShmRegion::create(name, 1024);
    auto region2 = ShmRegion::open(name);

    // Write from region1
    auto* data1 = static_cast<uint32_t*>(region1.data());
    data1[0] = 0xDEADBEEF;

    // Read from region2
    auto* data2 = static_cast<uint32_t*>(region2.data());
    EXPECT_EQ(data2[0], 0xDEADBEEF);

    // Write from region2
    data2[1] = 0xCAFEBABE;

    // Read from region1
    EXPECT_EQ(data1[1], 0xCAFEBABE);
}

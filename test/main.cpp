#include "gtest/gtest.h"

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();

    /*
     * TODO: the comprehensiveness and organization of additive tests
     * TODO: remove the RNG from the additive tests (there's not much point)
     */
}

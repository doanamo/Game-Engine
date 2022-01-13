/*
    Copyright (c) 2018-2021 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

int main(int argc, char* argv[])
{
    Logger::SetMode(Logger::Mode::UnitTests);
    Reflection::Initialize();
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

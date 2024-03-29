/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#include "Platform/FileSystem/FileHandle.hpp"
using namespace Platform;

FileHandle::FileHandle(const fs::path& path, OpenFlags::Type flags)
    : m_path(path)
    , m_flags(flags)
{
}

FileHandle::~FileHandle() = default;

const fs::path& FileHandle::GetPath() const
{
    return m_path;
}

std::string FileHandle::GetPathString() const
{
    return m_path.generic_string();
}

FileHandle::OpenFlags::Type FileHandle::GetFlags() const
{
    return m_flags;
}

bool FileHandle::IsReadOnly() const
{
    return (m_flags & OpenFlags::Read) && !(m_flags & OpenFlags::Write);
}

std::vector<uint8_t> FileHandle::ReadAsBinaryArray()
{
    std::vector<uint8_t> binary;
    binary.resize(GetSize());

    Seek(0, SeekMode::Begin);
    Read(reinterpret_cast<uint8_t*>(&binary[0]), GetSize());

    return binary;
}

std::string FileHandle::ReadAsTextString()
{
    std::string text;
    text.resize(GetSize());

    Seek(0, SeekMode::Begin);
    Read(reinterpret_cast<uint8_t*>(&text[0]), GetSize());

    return text;
}

/*
    Copyright (c) 2018-2022 Piotr Doan. All rights reserved.
    Software distributed under the permissive MIT License.
*/

#pragma once

#include "Platform/FileSystem/FileHandle.hpp"

/*
    File Depot

    Interface for file depot implementations that are mounted in file system.
*/

namespace Platform
{
    class FileDepot
    {
    public:
        enum class OpenFileErrors
        {
            EmptyFilePathArgument,
            InvalidFilePathArgument,
            InvalidOpenFlagsArgument,
            UnknownFileOpenError,
            FileNotFound,
            AccessDenied,
            TooManyHandles,
            FileTooLarge,
        };

        using OpenFileResult = Common::Result<std::unique_ptr<FileHandle>, OpenFileErrors>;
        using DirectoryContentList = std::vector<fs::path>;

    public:
        virtual ~FileDepot() = default;

        virtual OpenFileResult OpenFile(const fs::path& depotPath,
            const fs::path& requestedPath, FileHandle::OpenFlags::Type openFlags) = 0;
    };
}

#include "Record.h"

#include <DataStreams/NativeBlockInputStream.h>
#include <DataStreams/NativeBlockOutputStream.h>
/// #include <DataStreams/materializeBlock.h>
#include <Compression/CompressedReadBuffer.h>
#include <Compression/CompressedWriteBuffer.h>
#include <IO/ReadBufferFromMemory.h>
#include <IO/ReadHelpers.h>
#include <IO/WriteBufferFromVector.h>
#include <IO/WriteHelpers.h>


namespace DB
{
namespace DWAL
{
ByteVector Record::write(const Record & record, const bool compressed)
{
    ByteVector data{static_cast<size_t>((record.block.bytes() + 2) * 1.5)};
    WriteBufferFromVector wb{data};
    auto may_compressed_out = compressed ? CompressedWriteBuffer(wb) : wb;
    NativeBlockOutputStream output(may_compressed_out, 0, Block{});

    /// Write flags
    /// flags bits distribution
    /// [0-4] : Version
    /// [5-10] : OpCode
    /// [11-15] : Compression
    /// [16-63] : Reserved
    UInt64 flags = VERSION | (static_cast<UInt8>(record.op_code) << 5ul);
    if (compressed)
        flags |= COMPRESSED << 11ul;

    writeIntBinary(flags, wb);

    /// Data
    /// materializeBlockInplace(record.block);
    output.write(record.block);
    output.flush();

    /// Shrink to what has been written
    wb.finalize();
    return data;
}

RecordPtr Record::read(const char * data, size_t size)
{
    ReadBufferFromMemory rb{data, size};

    UInt64 flags = 0;
    readIntBinary(flags, rb);

    /// FIXME, more graceful version handling
    assert(Record::version(flags) == VERSION);

    std::unique_ptr<NativeBlockInputStream> input;
    std::unique_ptr<CompressedReadBuffer> compressed_in;
    if (Record::compression(flags))
    {
        compressed_in = std::make_unique<CompressedReadBuffer>(rb);
        input = std::make_unique<NativeBlockInputStream>(*compressed_in, 0);
    }
    else
        input = std::make_unique<NativeBlockInputStream>(rb, 0);

    return std::make_shared<Record>(Record::opcode(flags), input->read());
}
}
}

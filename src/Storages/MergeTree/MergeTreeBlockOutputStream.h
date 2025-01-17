#pragma once

#include "SequenceInfo.h"

#include <DataStreams/IBlockOutputStream.h>
#include <Storages/StorageInMemoryMetadata.h>


namespace DB
{

class Block;
class StorageMergeTree;


class MergeTreeBlockOutputStream : public IBlockOutputStream
{
public:
    MergeTreeBlockOutputStream(
        StorageMergeTree & storage_,
        const StorageMetadataPtr metadata_snapshot_,
        size_t max_parts_per_block_,
        ContextPtr context_)
        : storage(storage_)
        , metadata_snapshot(metadata_snapshot_)
        , max_parts_per_block(max_parts_per_block_)
        , context(context_)
    {
    }

    Block getHeader() const override;
    void write(const Block & block) override;
    void writePrefix() override;

    /// Daisy : starts
    void setSequenceInfo(const SequenceInfoPtr & seq_info_) { seq_info = seq_info_; }
    void setMissingSequenceRanges(SequenceRanges missing_seq_ranges_) { missing_seq_ranges.swap(missing_seq_ranges_); }

private:
    bool ignorePartBlock(Int32 parts, Int32 part_index) const;
    /// Daisy : ends

private:
    StorageMergeTree & storage;
    StorageMetadataPtr metadata_snapshot;
    size_t max_parts_per_block;
    ContextPtr context;

    /// Daisy : starts
    SequenceInfoPtr seq_info;
    SequenceRanges missing_seq_ranges;
    /// Daisy : ends
};

}

#ifndef OSCAR_CMD_CONFIG_H
#define OSCAR_CMD_CONFIG_H
#include <liboscar/StaticOsmCompleter.h>
#include "Benchmark.h"

namespace oscarcmd {

typedef enum { PS_NONE, PS_IDX_STORE, PS_COMPLETER, PS_DB, PS_GEO, PS_TAG, PS_GH, PS_ALL } PrintStatsSelection;

struct WD_base {
	WD_base() {}
	virtual ~WD_base() {}
	template<typename TCast>
	const TCast * as() const { return dynamic_cast<const TCast *>(this); }
	template<typename TCast>
	TCast * as() { return dynamic_cast<TCast*>(this); }
};

template<typename TValue>
struct WD_SingleValue: public WD_base {
	WD_SingleValue() {}
	WD_SingleValue(const TValue & v) : value(v) {}
	virtual ~WD_SingleValue() {}
	TValue value;
};

template<typename TValueFrist, typename TValueSecond>
struct WD_PairValue: public WD_base {
	WD_PairValue() {}
	WD_PairValue(const TValueFrist & first, const TValueSecond & second) : first(first), second(second) {}
	virtual ~WD_PairValue() {}
	TValueFrist first;
	TValueSecond second;
};

typedef WD_PairValue<int, int> WD_SelectTextCompleter;
typedef WD_SingleValue<int> WD_SelectGeoCompleter;

struct WD_PrintStats: public WD_base {
	WD_PrintStats(PrintStatsSelection printStats) : printStats(printStats), out(&std::cout) {}
	WD_PrintStats(PrintStatsSelection printStats, const std::string & fileName) : printStats(printStats), fileName(fileName), out(0) {}
	virtual ~WD_PrintStats() {}
	PrintStatsSelection printStats;
	std::string fileName;
	std::ostream * out;
};

struct WD_PrintStatsSingle: WD_PrintStats {
	WD_PrintStatsSingle(PrintStatsSelection printStats, uint32_t what) : WD_PrintStats(printStats), what(what) {}
    virtual ~WD_PrintStatsSingle() {}
	uint32_t what;
};

struct WD_DumpAllItemTagsWithInheritedTags: public WD_base {
	std::string keyfile;
	std::string outfile;
	WD_DumpAllItemTagsWithInheritedTags(const std::string & keyfile, const std::string & outfile) :
	keyfile(keyfile),
	outfile(outfile)
	{}
	virtual ~WD_DumpAllItemTagsWithInheritedTags() {}
};

typedef WD_SingleValue<uint32_t> WD_GhId2StoreId;
typedef WD_SingleValue<std::string> WD_PrintCQRDataSize;
typedef WD_SingleValue<std::string> WD_PrintCTCStorageStats;
typedef WD_SingleValue<std::string> WD_PrintCTCSelectiveStorageStats;
typedef WD_SingleValue<std::string> WD_PrintPaperStatsDb;
typedef WD_SingleValue<std::string> WD_PrintPaperStatsGh;
typedef WD_SingleValue<uint32_t> WD_DumpIndex;
typedef WD_SingleValue<uint32_t> WD_DumpItem;
typedef WD_SingleValue<bool> WD_DumpAllItems;
typedef WD_SingleValue<uint32_t> WD_DumpGhRegion;
typedef WD_SingleValue<uint32_t> WD_DumpGhCell;
typedef WD_SingleValue<uint32_t> WD_DumpGhRegionItems;
typedef WD_SingleValue<uint32_t> WD_DumpGhRegionChildren;
typedef WD_SingleValue<uint32_t> WD_DumpGhCellItems;
typedef WD_SingleValue<uint32_t> WD_DumpGhCellParents;
typedef WD_SingleValue<std::string> WD_DumpGh;
typedef WD_SingleValue< std::vector<uint32_t> > WD_DumpGhPath;
typedef WD_SingleValue<std::string> WD_DumpKeyStringTable;
typedef WD_SingleValue<std::string> WD_DumpValueStringTable;
typedef WD_SingleValue<std::string> WD_DumpItemTags;
typedef WD_SingleValue<bool> WD_InteractiveFull;
typedef WD_SingleValue<std::string> WD_ConsistencyCheck;

struct WD_InteractivePartial: public WD_base {
	WD_InteractivePartial(int printNumResults, uint32_t seek) :
	printNumResults(printNumResults), seek(seek) {}
	virtual ~WD_InteractivePartial() {}
	int printNumResults;
	uint32_t seek;
};

struct WD_InteractiveSimple: public WD_base {
	WD_InteractiveSimple(int printNumResults, uint32_t minStrLen, uint32_t maxResultSetSize) :
	printNumResults(printNumResults), minStrLen(minStrLen), maxResultSetSize(maxResultSetSize) {}
	virtual ~WD_InteractiveSimple() {}
	int printNumResults;
	uint32_t minStrLen;
	uint32_t maxResultSetSize;
};

struct WD_CreateCompletionStrings: WD_base {
	WD_CreateCompletionStrings(const std::string & outFileName, uint32_t count, bool simulate = false) :
	outFileName(outFileName), count(count), simulate(simulate) {}
	virtual ~WD_CreateCompletionStrings() {}
	std::string outFileName;
	uint32_t count;
	bool simulate;
};

struct WD_CompletionBase: public WD_base {
	WD_CompletionBase(int printNumResults) : printNumResults(printNumResults) {}
	virtual ~WD_CompletionBase() {}
	int printNumResults;
};

struct WD_CompleteStringBase: public WD_CompletionBase {
	WD_CompleteStringBase(const std::string & str, int printNumResults) :
	WD_CompletionBase(printNumResults), str(str) {}
	virtual ~WD_CompleteStringBase() {}
	std::string str;
};

struct WD_CompleteStringPartial: public WD_CompleteStringBase {
	WD_CompleteStringPartial(const std::string & str, int printNumResults, uint32_t seek = 0) :
	WD_CompleteStringBase(str, printNumResults), seek(seek) {}
	virtual ~WD_CompleteStringPartial() {}
	uint32_t seek;
};

struct WD_CompleteStringSimple: public WD_CompleteStringBase {
	WD_CompleteStringSimple(const std::string & str, int printNumResults, uint32_t minStrLen, uint32_t maxResultSetSize) : 
	WD_CompleteStringBase(str, printNumResults), minStrLen(minStrLen), maxResultSetSize(maxResultSetSize) {}
	virtual ~WD_CompleteStringSimple() {}
	uint32_t minStrLen;
	uint32_t maxResultSetSize;
};

struct WD_CompleteStringFull: public WD_CompleteStringBase {
	WD_CompleteStringFull(const std::string & str, int printNumResults) :
	WD_CompleteStringBase(str, printNumResults) {}
	virtual ~WD_CompleteStringFull() {}
};

struct WD_CompleteStringClustered: public WD_CompleteStringBase {
	WD_CompleteStringClustered(const std::string & str, int printNumResults) : 
	WD_CompleteStringBase(str, printNumResults) {}
	virtual ~WD_CompleteStringClustered() {}
};

struct WD_CompleteStringClusteredTreedCqr: public WD_CompleteStringClustered {
	WD_CompleteStringClusteredTreedCqr(const std::string & str, int printNumResults) : 
	WD_CompleteStringClustered(str, printNumResults) {}
	virtual ~WD_CompleteStringClusteredTreedCqr() {}
};

struct WD_CompleteFromFileBase: public WD_CompletionBase {
	WD_CompleteFromFileBase(int printNumResults, const std::string & fileName) :
	WD_CompletionBase(printNumResults), fileName(fileName)
	{}
	virtual ~WD_CompleteFromFileBase() {}
	std::string fileName;
};

struct WD_CompleteFromFilePartial: public WD_CompleteFromFileBase {
	WD_CompleteFromFilePartial(int printNumResults, const std::string & fileName, uint32_t seek) :
	WD_CompleteFromFileBase(printNumResults, fileName), seek(seek) {}
	virtual ~WD_CompleteFromFilePartial() {}
	uint32_t seek;
};

struct WD_CompleteFromFileSimple: public WD_CompleteFromFileBase {
	WD_CompleteFromFileSimple(int printNumResults, const std::string & fileName, uint32_t minStrLen, uint32_t maxResultSetSize) :
	WD_CompleteFromFileBase(printNumResults, fileName), minStrLen(minStrLen), maxResultSetSize(maxResultSetSize) {}
	virtual ~WD_CompleteFromFileSimple() {}
	uint32_t minStrLen;
	uint32_t maxResultSetSize;
};

struct WD_CompleteFromFileFull: public WD_CompleteFromFileBase {
	WD_CompleteFromFileFull(int printNumResults, const std::string & fileName) :
	WD_CompleteFromFileBase(printNumResults, fileName) {}
	virtual ~WD_CompleteFromFileFull() {}
};

struct WD_CompleteFromFileClustered: public WD_CompleteFromFileBase {
	WD_CompleteFromFileClustered(int printNumResults, const std::string & fileName) :
	WD_CompleteFromFileBase(printNumResults, fileName) {}
	virtual ~WD_CompleteFromFileClustered() {}
};

struct WD_CompleteFromFileClusteredTreedCqr: public WD_CompleteFromFileClustered {
	WD_CompleteFromFileClusteredTreedCqr(int printNumResults, const std::string & fileName) :
	WD_CompleteFromFileClustered(printNumResults, fileName) {}
	virtual ~WD_CompleteFromFileClusteredTreedCqr() {}
};

struct WD_SymDiffItemsCompleters: public WD_CompleteStringBase {
	WD_SymDiffItemsCompleters(int printNumResults, const std::string & str, uint32_t completer1, uint32_t completer2) :
	WD_CompleteStringBase(str, printNumResults), completer1(completer1), completer2(completer2) {}
	uint32_t completer1;
	uint32_t completer2;
};

struct WD_Benchmark: public WD_base {
	WD_Benchmark(const Benchmarker::Config & cfg) : config(cfg) {}
	virtual ~WD_Benchmark() {}
	Benchmarker::Config config;
};

class Config {
public:
	struct WorkItem {
		typedef enum {
			SELECT_TEXT_COMPLETER, SELECT_GEO_COMPLETER, PRINT_SELECTED_TEXT_COMPLETER, PRINT_SELECTED_GEO_COMPLETER,
			DUMP_INDEX, DUMP_ITEM, DUMP_ALL_ITEMS, DUMP_GH_REGION, DUMP_GH_CELL, DUMP_GH_CELL_PARENTS, DUMP_GH_CELL_ITEMS, DUMP_GH_REGION_ITEMS, DUMP_GH, DUMP_GH_PATH,
			DUMP_KEY_STRING_TABLE, DUMP_VALUE_STRING_TABLE, DUMP_ITEM_TAGS, DUMP_GH_REGION_CHILDREN,
			GH_ID_2_STORE_ID,
			PRINT_STATS, PRINT_STATS_SINGLE, PRINT_PAPER_STATS_DB, PRINT_PAPER_STATS_GH, PRINT_CTC_STATS, PRINT_CTC_SELECTIVE_STATS, PRINT_CQR_DATA_SIZE, DUMP_ALL_ITEM_TAGS_WITH_INHERITED_TAGS,
			LIST_COMPLETERS,
			INTERACTIVE_PARTIAL, INTERACTIVE_SIMPLE, INTERACTIVE_FULL,
			COMPLETE_STRING_PARTIAL, COMPLETE_STRING_SIMPLE, COMPLETE_STRING_FULL, COMPLETE_STRING_CLUSTERED, COMPLETE_STRING_CLUSTERED_TREED_CQR,
			COMPLETE_FROM_FILE_PARTIAL, COMPLETE_FROM_FILE_SIMPLE, COMPLETE_FROM_FILE_FULL, COMPLETE_FROM_FILE_CLUSTERED, COMPLETE_FROM_FILE_CLUSTERED_TREED_CQR,
			SYMDIFF_ITEMS_COMPLETERS,
			CREATE_COMPLETION_STRINGS, BENCHMARK, CONSISTENCY_CHECK
			} Type;
		Type type;
		std::shared_ptr<WD_base> data;
		///takes ownership of d
		WorkItem(Type t, WD_base * d) : type(t), data(d) {}
		WorkItem(Type t) : type(t), data(0) {}
		~WorkItem() {}
	};
private:
	enum {PRT_OK, PRT_HELP};
public:
	std::string inFileName;
	//no the todo queue
	std::vector<WorkItem> workItems;
private:
	int parseSingleArg(int argc, char ** argv, int & i, int & printNumResults, std::string & completionString);
public:
	Config() {}
	bool fromCommandline(int argc, char ** argv);
	bool valid();
	static void printHelp();
};


}//end namespace

#endif
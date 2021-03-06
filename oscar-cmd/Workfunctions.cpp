#include "Workfunctions.h"
#include <unordered_set>
#include "Config.h"
#include "ConsistencyCheckers.h"
#include "GeoHierarchyPrinter.h"
#include "LiveCompleter.h"
#include <sserialize/strings/unicode_case_functions.h>
#include <sserialize/stats/ProgressInfo.h>

namespace oscarcmd {

std::string statsFileNameSuffix(uint32_t which) {
	switch (which) {
	case PS_ALL:
		return "all";
	case PS_IDX_STORE:
		return "idxstore";
	case PS_COMPLETER:
		return "comp";
	case PS_DB:
		return "db";
	case PS_GEO:
		return "geo";
	case PS_TAG:
		return "tag";
	case PS_GH:
		return "gh";
	default:
		return "nix";
	}
}

void doPrintStats(std::ostream & out, liboscar::Static::OsmCompleter & completer, uint32_t which) {
	switch (which) {
	case PS_ALL:
		completer.printStats(out);
		break;
	case PS_IDX_STORE:
		completer.indexStore().printStats(out);
		break;
	case PS_COMPLETER:
		for(uint32_t i(0), s(completer.textSearch().size(liboscar::TextSearch::ITEMS)); i < s; ++i) {
			completer.textSearch().get<liboscar::TextSearch::ITEMS>(i).printStats(out);
		}
		for(uint32_t i(0), s(completer.textSearch().size(liboscar::TextSearch::GEOHIERARCHY)); i < s; ++i) {
			completer.textSearch().get<liboscar::TextSearch::GEOHIERARCHY>(i).printStats(out);
		}
		for(uint32_t i(0), s(completer.textSearch().size(liboscar::TextSearch::GEOCELL)); i < s; ++i) {
			completer.textSearch().get<liboscar::TextSearch::GEOCELL>(i).printStats(out);
		}
		break;
	case PS_DB:
		completer.store().printStats(out);
	case PS_GEO:
		out << completer.geoCompleter()->describe();
		break;
	case PS_TAG:
		completer.tagStore().printStats(out);
		break;
	case PS_GH:
		completer.store().geoHierarchy().printStats(out, completer.indexStore());
		break;
	default:
		break;
	}
}

void Worker::listCompleters() {
	if (completer.textSearch().hasSearch(liboscar::TextSearch::ITEMS)) {
		std::cout << "Items completers:\n"; 
		for(uint32_t i(0), s(completer.textSearch().size(liboscar::TextSearch::ITEMS)); i < s; ++i) {
			std::cout << "\t" << i << "=" << completer.textSearch().get<liboscar::TextSearch::ITEMS>(i).getName() << "\n";
		}
	}
	if (completer.textSearch().hasSearch(liboscar::TextSearch::GEOHIERARCHY)) {
		std::cout << "GeoHierarchy completers:\n"; 
		for(uint32_t i(0), s(completer.textSearch().size(liboscar::TextSearch::GEOHIERARCHY)); i < s; ++i) {
			std::cout << "\t" << i << "=" << completer.textSearch().get<liboscar::TextSearch::GEOHIERARCHY>(i).getName() << "\n";
		}
	}
	if (completer.textSearch().hasSearch(liboscar::TextSearch::GEOCELL)) {
		std::cout << "GeoCell completers:\n"; 
		for(uint32_t i(0), s(completer.textSearch().size(liboscar::TextSearch::GEOCELL)); i < s; ++i) {
			std::cout << "\t" << i << "=" << completer.textSearch().get<liboscar::TextSearch::GEOCELL>(i).trie().getName() << "\n";
		}
	}
	std::cout << std::flush;
}


void Worker::printStats(const WD_PrintStats & data) {
	if (data.printStats != PS_NONE) {
		if (data.fileName.size()) {
			std::ofstream of;
			of.open(data.fileName + "." + statsFileNameSuffix(data.printStats) + "/stats.txt");
			if (!of.is_open()) {
				throw std::runtime_error("Failed to open " + data.fileName + "/stats.txt");
			}
			else {
				doPrintStats(of, completer, data.printStats);
				of.close();
			}
		}
		else {
			doPrintStats(*data.out, completer, data.printStats);
		}
	}
}

void Worker::printCQRDataSize(const WD_PrintCQRDataSize& data) {
	std::string qstr = data.value;
	const sserialize::Static::spatial::GeoHierarchy & gh = completer.store().geoHierarchy();
	const sserialize::Static::ItemIndexStore & idxStore = completer.indexStore();
	sserialize::Static::CellTextCompleter cmp( completer.textSearch().get<liboscar::TextSearch::Type::GEOCELL>() );
	sserialize::StringCompleter::QuerryType qt = sserialize::StringCompleter::normalize(qstr);
	sserialize::Static::CellTextCompleter::Payload::Type t = cmp.typeFromCompletion(qstr, qt);
	sserialize::ItemIndex fmIdx = idxStore.at( t.fmPtr() );
	sserialize::ItemIndex pmIdx = idxStore.at( t.pPtr() );
	sserialize::CellQueryResult r(fmIdx, pmIdx, t.pItemsPtrBegin(), gh, idxStore);
	std::vector<uint32_t> idxDataSizes;
	std::vector<uint32_t> idxSizes;
	idxDataSizes.reserve(r.cellCount());
	idxSizes.reserve(r.cellCount());
	sserialize::UByteArrayAdapter::OffsetType idxDataSize = 0;
	sserialize::UByteArrayAdapter::OffsetType idxDataSizeForGhCellIdx = 0;
	for(sserialize::CellQueryResult::const_iterator it(r.begin()), end(r.end()); it != end; ++it) {
		const sserialize::ItemIndex & idx = *it;
		idxDataSize += idx.getSizeInBytes();
		idxDataSizes.push_back(idx.getSizeInBytes());
		idxSizes.push_back(idx.size());
		idxDataSizeForGhCellIdx += idxStore.dataSize( gh.cellItemsPtr( it.cellId() ) );
	}
	std::sort(idxDataSizes.begin(), idxDataSizes.end());
	std::sort(idxSizes.begin(), idxSizes.end());
	sserialize::UByteArrayAdapter::OffsetType totalDataSize = 0;
	totalDataSize = idxDataSize+fmIdx.getSizeInBytes() + pmIdx.getSizeInBytes();
	totalDataSize += t.data().data().size();
	
	std::cout << "Index data size for cqr: " << idxDataSize << std::endl;
	std::cout << "Index data size of gh-cell idexes: " << idxDataSizeForGhCellIdx << std::endl;
	std::cout << "Cell match index size: " << fmIdx.getSizeInBytes() + pmIdx.getSizeInBytes() << std::endl;
	std::cout << "Total data size: " << totalDataSize << std::endl;
	std::cout << "Idx data sizes:\n";
	std::cout << "\tmean:" << sserialize::statistics::mean(idxDataSizes.begin(), idxDataSizes.end(), (double)0.0) << "\n";
	std::cout << "\tmedian: " << idxDataSizes.at(idxDataSizes.size()/2) << "\n";
	std::cout << "\tmin: " << idxDataSizes.front() << "\n";
	std::cout << "\tmax: " << idxDataSizes.back() << "\n";
	std::cout << "Idx sizes:\n";
	std::cout << "\tmean:" << sserialize::statistics::mean(idxSizes.begin(), idxSizes.end(), (double)0.0) << "\n";
	std::cout << "\tmedian: " << idxSizes.at(idxSizes.size()/2) << "\n";
	std::cout << "\tmin: " << idxSizes.front() << "\n";
	std::cout << "\tmax: " << idxSizes.back() << "\n";
	std::cout << std::flush;
}

void Worker::printCTCSelectiveStorageStats(const WD_PrintCTCSelectiveStorageStats& data) {
	typedef sserialize::Static::CellTextCompleter CTC;
	sserialize::StringCompleter::QuerryType qt = sserialize::StringCompleter::QT_NONE;
	if (data.value == "exact") {
		qt = sserialize::StringCompleter::QT_EXACT;
	}
	else if (data.value == "prefix") {
		qt = sserialize::StringCompleter::QT_PREFIX;
	}
	else if (data.value == "suffix") {
		qt = sserialize::StringCompleter::QT_SUFFIX;
	}
	else if (data.value == "substring") {
		qt = sserialize::StringCompleter::QT_SUBSTRING;
	}
	else {
		std::cout << "Invalid type specified" << std::endl;
		return;
	}
	const sserialize::Static::ItemIndexStore & idxStore = completer.indexStore();
	auto ft = completer.textSearch().get<liboscar::TextSearch::GEOCELL>().trie().as<CTC::FlatTrieType>();
	if (ft) {
		std::unordered_set<uint32_t> idxIds;
		sserialize::UByteArrayAdapter::OffsetType treeStorageSize = 0;
		sserialize::ProgressInfo pinfo;
		const CTC::FlatTrieType::TrieType & ftp = ft->trie();
		pinfo.begin(ftp.size(), "Gathering CTC storage stats");
		for(uint32_t i=0, s(ftp.size()); i < s; ++i) {
			CTC::Payload p( ftp.at(i) );
			CTC::Payload::Type t( p.type(qt) );
			if (t.valid()) {
				uint32_t pIdxSize = idxStore.idxSize(t.pPtr());
				treeStorageSize += p.typeData(qt).size();
				
				idxIds.insert(t.fmPtr());
				idxIds.insert(t.pPtr());
				auto it = t.pItemsPtrBegin();
				for(uint32_t i(0); i < pIdxSize; ++i, ++it) {
					idxIds.insert(*it);
				}
			}
			pinfo(i);
		}
		pinfo.end();
		std::cout << "Number of different indexes for the tree-data: " << idxIds.size() << "\n";
		for(uint32_t i(0), s(completer.store().geoHierarchy().cellSize()); i < s; ++i) {
			idxIds.insert(completer.store().geoHierarchy().cellItemsPtr(i));
		}
		sserialize::UByteArrayAdapter::OffsetType idxDataSize = 0;
		for(uint32_t x : idxIds) {
			idxDataSize += idxStore.dataSize(x);
		}
		idxIds.clear();

		std::cout << "Tree value data size: " << treeStorageSize << "\n";
		std::cout << "Index data size: " << idxDataSize << "\n";
		std::cout << "Total minimum data size: " << sserialize::prettyFormatSize(treeStorageSize+idxDataSize) << std::endl;
	}
	else {
		std::cout << "CTC is unsupported" << std::endl;
	}
}

void Worker::printCTCStorageStats(const WD_PrintCTCStorageStats & data) {
	if (!completer.textSearch().hasSearch(liboscar::TextSearch::GEOCELL))
		return;

	std::ofstream outfile;
	outfile.open(data.value);
	if (!outfile.is_open()) {
		throw std::runtime_error("Failed to open file: " + data.value);
	}

	typedef sserialize::Static::CellTextCompleter CTC;
	struct Stats {
		std::unordered_map<uint32_t, std::vector<sserialize::UByteArrayAdapter::OffsetType> > index;
		std::vector<sserialize::UByteArrayAdapter::OffsetType> tree;
		std::unordered_set<uint32_t> fmPmIndexIds;
		Stats() : tree(1024, 0) {
			index[sserialize::StringCompleter::QT_EXACT] = tree;
			index[sserialize::StringCompleter::QT_PREFIX] = tree;
			index[sserialize::StringCompleter::QT_SUFFIX] = tree;
			index[sserialize::StringCompleter::QT_SUBSTRING] = tree;
		};
	} stats;
	const sserialize::Static::ItemIndexStore & idxStore = completer.indexStore();
	auto ft = completer.textSearch().get<liboscar::TextSearch::GEOCELL>().trie().as<CTC::FlatTrieType>();
	if (ft) {
		std::cout << "Gathering CTC storage stats" << std::endl;
		struct MyRec {
			const sserialize::Static::ItemIndexStore & idxStore;
			Stats & stats;
			const CTC::FlatTrieType::TrieType & ftp;
			uint32_t longestString;
			void calc(const sserialize::Static::UnicodeTrie::FlatTrieBase::Node & node, int strlen) {
				uint32_t nodeId = node.id();
				
				stats.tree.at(strlen+1) += ftp.payloads().dataSize(nodeId);
				
				CTC::Payload p( ftp.at(nodeId) );
				for(auto & x : stats.index) {
					if (p.types() & x.first) {
						CTC::Payload::Type t( p.type((sserialize::StringCompleter::QuerryType) x.first) );
						stats.fmPmIndexIds.insert(t.fmPtr());
						stats.fmPmIndexIds.insert(t.pPtr());
						sserialize::UByteArrayAdapter::OffsetType tmp = 0;
						tmp += idxStore.dataSize(t.fmPtr()) + idxStore.dataSize(t.pPtr());
						auto it = t.pItemsPtrBegin();
						for(uint32_t i(0), s(idxStore.idxSize(t.pPtr())); i < s; ++i, ++it) {
							tmp += idxStore.dataSize(*it);
						}
						x.second.at(strlen+1) += tmp;
					}
				}
				longestString = std::max<uint32_t>(longestString, strlen+1);
				
				uint32_t nodePathStrLen = ftp.strSize(nodeId);
				for(auto child : node) {
					calc(child, nodePathStrLen);
				}
			}
		};
		MyRec myrec(
			{
				.idxStore = idxStore,
				.stats = stats,
				.ftp = ft->trie(),
				.longestString = 0
			}
		);
		myrec.calc(myrec.ftp.root(), -1);
		for(uint32_t i(0); i <= myrec.longestString; ++i) {
			outfile << stats.tree.at(i) << ";";
			outfile << stats.index.at(sserialize::StringCompleter::QT_EXACT).at(i) << ";";
			outfile << stats.index.at(sserialize::StringCompleter::QT_PREFIX).at(i) << ";";
			outfile << stats.index.at(sserialize::StringCompleter::QT_SUFFIX).at(i) << ";";
			outfile << stats.index.at(sserialize::StringCompleter::QT_SUBSTRING).at(i) << "\n";
		}
		sserialize::UByteArrayAdapter::OffsetType fmPmIndexDataSize = 0;
		sserialize::UByteArrayAdapter::OffsetType cellIndexSizes = 0;
		for(uint32_t x : stats.fmPmIndexIds) {
			fmPmIndexDataSize += completer.indexStore().dataSize(x);
		}
		for(uint32_t i(0), s(completer.store().geoHierarchy().cellSize()); i < s; ++i) {
			stats.fmPmIndexIds.insert(completer.store().geoHierarchy().cellItemsPtr(i));
		}
		
		for(uint32_t x : stats.fmPmIndexIds) {
			cellIndexSizes += completer.indexStore().dataSize(x);
		}
		std::cout << "Data size of fm and pm cell indexes: " << fmPmIndexDataSize << std::endl;
		std::cout << "With gh cell indexes: " << cellIndexSizes << std::endl;
	}
	else {
		std::cout << "CTC is unsupported" << std::endl;
	}
}

void Worker::printPaperStatsDb(const WD_PrintPaperStatsDb & data) {

	std::ifstream file;
	file.open(data.value);
	if (!file.is_open()) {
		throw std::runtime_error("Failed to open key file: " + data.value);
	}

	uint64_t rawValueSizes = 0;
	uint64_t flattenedValueSizes = 0;
	
	std::unordered_set<uint32_t> keyIds;
	std::unordered_set<uint32_t> regionValueIds;
		
	while (!file.eof()) {
		std::string key;
		std::getline(file, key);
		if (key.size()) {
			keyIds.insert(completer.store().keyStringTable().find(key));
		}
	}
	file.close();

	auto vst = completer.store().valueStringTable();
	auto gh = completer.store().geoHierarchy();
	auto idxStore = completer.indexStore();
	auto cellPtrs = gh.cellPtrs();
	sserialize::ProgressInfo pinfo;
	pinfo.begin(completer.store().size(), "Analyzing");
	for(uint32_t i(0), s(completer.store().size()); i < s; ++i) {
		regionValueIds.clear();
		auto item = completer.store().at(i);
		for(uint32_t j(0), js(item.size()); j < js; ++j) {
			if (keyIds.count(item.keyId(j))) {
				rawValueSizes += vst.strSize(item.valueId(j));
			}
		}
		for(uint32_t cellId : item.cells()) {
			for(uint32_t j(gh.cellParentsBegin(cellId)), js(gh.cellParentsEnd(cellId)); j < js; ++j) {
				uint32_t regionId = gh.ghIdToStoreId(cellPtrs.at(j));
				auto ritem = completer.store().at(regionId);
				for(uint32_t k(0), ks(ritem.size()); k < ks; ++k) {
					if (keyIds.count(ritem.keyId(k))) {
						regionValueIds.insert(ritem.valueId(k));
					}
				}
			}
		}
		for(uint32_t x : regionValueIds) {
			flattenedValueSizes += vst.strSize(x);
		}
		pinfo(i);
	}
	pinfo.end();
	
	std::cout << "item value size for selected keys: " << rawValueSizes << "\n";
	std::cout << "inherited value size for selected keys: " << flattenedValueSizes << "\n";
	std::cout << "inherited+item value size for selected keys: " << flattenedValueSizes+rawValueSizes << "\n";
}

void Worker::printPaperStatsGh(const WD_PrintPaperStatsGh& data) {
	std::ofstream outFile;
	outFile.open(data.value);
	if (!outFile.is_open()) {
		throw std::runtime_error("Could not open outfile");
		return;
	}
	const sserialize::Static::spatial::GeoHierarchy & gh = completer.store().geoHierarchy();
	std::vector< std::pair<uint32_t, uint32_t> > cellItemSizes(gh.cellSize());
	for(uint32_t i(0), s(gh.cellSize()); i < s; ++i) {
		uint32_t cellItemCount = gh.cellItemsCount(i);
		auto & x = cellItemSizes[i];
		x.first = cellItemCount;
		x.second = i;
	}
	std::sort(cellItemSizes.begin(), cellItemSizes.end());
	for(auto x : cellItemSizes) {
		outFile << x.first << ";" << x.second << "\n";
	}
	outFile.close();
}

void Worker::dumpAllItemTagsWithInheritedTags(const WD_DumpAllItemTagsWithInheritedTags& data) {

	std::unordered_set<uint32_t> keyIds;
	std::unordered_set<uint32_t> regionValueIds;
	{
		std::ifstream file;
		file.open(data.keyfile);
		if (!file.is_open()) {
			throw std::runtime_error("Failed to open key file: " + data.keyfile);
		}
		while (!file.eof()) {
			std::string key;
			std::getline(file, key);
			if (key.size()) {
				keyIds.insert(completer.store().keyStringTable().find(key));
			}
		}
		file.close();
	}
	std::ofstream file;
	file.open(data.outfile);

	auto vst = completer.store().valueStringTable();
	auto gh = completer.store().geoHierarchy();
	auto idxStore = completer.indexStore();
	auto cellPtrs = gh.cellPtrs();
	sserialize::ProgressInfo pinfo;
	pinfo.begin(completer.store().size(), "Writing");
	for(uint32_t i(0), s(completer.store().size()); i < s; ++i) {
		file << "itemid:" << i;
		regionValueIds.clear();
		auto item = completer.store().at(i);
		for(uint32_t cellId : item.cells()) {
			for(uint32_t j(gh.cellParentsBegin(cellId)), js(gh.cellParentsEnd(cellId)); j < js; ++j) {
				uint32_t regionId = gh.ghIdToStoreId(cellPtrs.at(j));
				auto ritem = completer.store().at(regionId);
				for(uint32_t k(0), ks(ritem.size()); k < ks; ++k) {
					if (keyIds.count(ritem.keyId(k))) {
						regionValueIds.insert(ritem.valueId(k));
					}
				}
			}
		}
		for(uint32_t x : regionValueIds) {
			file << vst.at(x);
		}
		for(uint32_t j(0), js(item.size()); j < js; ++j) {
			if (keyIds.count(item.keyId(j))) {
				file << item.value(j);
			}
		}
		file.put('\0');
		pinfo(i);
	}
	pinfo.end();
	
}

void Worker::selectTextCompleter(WD_SelectTextCompleter & d) {
	if(!completer.setTextSearcher((liboscar::TextSearch::Type)d.first, d.second)) {
		std::cout << "Failed to set selected completer: " << (uint32_t)d.first << ":" << (uint32_t)d.second << std::endl;
	}
}

void Worker::selectGeoCompleter(WD_SelectGeoCompleter & d) {
	if (!completer.setGeoCompleter(d.value)) {
		std::cout << "Failed to set seleccted geo completer: " << d.value << std::endl;
	}
}

void Worker::printSelectedTextCompleter() {
	if (completer.textSearch().hasSearch(liboscar::TextSearch::ITEMS) ) {
		std::cout << "Selected items text completer: " << completer.textSearch().get<liboscar::TextSearch::ITEMS>().getName() << std::endl;
	}
	if (completer.textSearch().hasSearch(liboscar::TextSearch::GEOHIERARCHY)) {
		std::cout << "Selected geohierarchy text completer: " << completer.textSearch().get<liboscar::TextSearch::GEOHIERARCHY>().getName() << std::endl;
	}
}

void Worker::printSelectedGeoCompleter() {
	std::cout << "Selected Geo completer: " << completer.geoCompleter()->describe() << std::endl; 
}

void Worker::consistencyCheck(WD_ConsistencyCheck & d) {
	if (d.value == "index") {
		if (!ConsistencyChecker::checkIndex(completer.indexStore())) {
			std::cout << "Index Store is BROKEN" << std::endl;
		}
		else {
			std::cout << "Index Store is OK" << std::endl;
		}
	}
	else if (d.value == "store") {
		if (!ConsistencyChecker::checkStore(completer.store())) {
			std::cout << "Store is BROKEN" << std::endl;
		}
		else {
			std::cout << "Store is OK" << std::endl;
		}
	}
	else if (d.value == "gh") {
		if (!ConsistencyChecker::checkGh(completer.store(), completer.indexStore())) {
			std::cout << "GeoHierarchy is BROKEN" << std::endl;
		}
		else {
			std::cout << "GeoHierarchy is OK" << std::endl;
		}
	}
}

void Worker::ghId2StoreId(WD_GhId2StoreId& d) {
	std::cout << "GhId2storeId: " << d.value << "->" << completer.store().geoHierarchy().ghIdToStoreId(d.value) << std::endl;
}

void Worker::dumpIndex(WD_DumpIndex& d) {
	std::cout << completer.indexStore().at(d.value) << std::endl;
}

void Worker::dumpItem(WD_DumpItem& d) {
	completer.store().at(d.value).print(std::cout, true);
	std::cout << std::endl;
}

void Worker::dumpAllItems(WD_DumpAllItems& d) {
	for(uint32_t i(0), s(completer.store().size()); i < s; ++i) {
		completer.store().at(i).print(std::cout, d.value);
		std::cout << std::endl;
	}
}

void Worker::dumpGhRegion(WD_DumpGhRegion& d) {
	std::cout << completer.store().geoHierarchy().regionFromStoreId(d.value) << std::endl;
}

void Worker::dumpGhCell(WD_DumpGhCell& d) {
	std::cout << completer.store().geoHierarchy().cell(d.value) << std::endl;
}

void Worker::dumpGhCellParents(WD_DumpGhCellParents& d) {
	const sserialize::Static::spatial::GeoHierarchy & gh = completer.store().geoHierarchy();
	sserialize::Static::spatial::GeoHierarchy::Cell c = gh.cell(d.value);
	for(uint32_t i(0), s(c.parentsSize()); i < s; ++i) {
		uint32_t parentGhId = c.parent(i);
		uint32_t parentStoreId = gh.ghIdToStoreId(parentGhId);
		completer.store().at(parentStoreId).print(std::cout, false);
		std::cout << "\n";
	}
}

void Worker::dumpGhCellItems(WD_DumpGhCellItems& d) {
	sserialize::ItemIndex cItems = completer.indexStore().at(completer.store().geoHierarchy().cell(d.value).itemPtr());
	for(uint32_t i : cItems) {
		completer.store().at(i).print(std::cout, false);
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

void Worker::dumpGhRegionChildren(WD_DumpGhRegionChildren& d) {
	sserialize::Static::spatial::GeoHierarchy::Region r;
	if (d.value == 0xFFFFFFFF) {
		r = completer.store().geoHierarchy().rootRegion();
		std::cout << "Root-region has the following children: " << std::endl;
	}
	else {
		r = completer.store().geoHierarchy().region(d.value);
		completer.store().at(r.storeId()).print(std::cout, false);
		std::cout << "\nhas the following children:" << std::endl;
	}
	
	for(uint32_t i(0), s(r.childrenSize()); i < s; ++i) {
		uint32_t childRegionId = r.child(i);
		uint32_t childStoreId = completer.store().geoHierarchy().ghIdToStoreId(childRegionId);
		completer.store().at(childStoreId).print(std::cout, false);
		std::cout << std::endl;
	}
}

void Worker::dumpGhRegionItems(WD_DumpGhRegionItems& d) {
	sserialize::ItemIndex rItems = completer.indexStore().at(completer.store().geoHierarchy().region(d.value).itemsPtr());
	for(uint32_t i : rItems) {
		completer.store().at(i).print(std::cout, false);
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

void Worker::dumpGh(WD_DumpGh & d) {
	std::ofstream outPrintFile;
	outPrintFile.open("children" + d.value);
	if (outPrintFile.is_open()) {
		GeoHierarchyPrinter printer;
		std::stringstream tmp;
		printer.printChildrenWithNames(tmp, completer.store());
		outPrintFile << tmp.rdbuf();
		outPrintFile.close();
	}
	else {
		throw std::runtime_error("Failed to open printfile: children " + d.value);
	}
	outPrintFile.open("parents" + d.value);
	if (outPrintFile.is_open()) {
		GeoHierarchyPrinter printer;
		std::stringstream tmp;
		printer.printParentsWithNames(tmp, completer.store());
		outPrintFile << tmp.rdbuf();
		outPrintFile.close();
	}
	else {
		throw std::runtime_error("Failed to open printfile: parents " + d.value);
	}
}

void Worker::dumpGhPath(WD_DumpGhPath & d) {
	sserialize::Static::spatial::GeoHierarchy::Region r = completer.store().geoHierarchy().rootRegion();
	for(std::vector<uint32_t>::const_iterator it(d.value.begin()), end(d.value.end()); it != end; ++it) {
		if (*it < r.childrenSize()) {
			r = completer.store().geoHierarchy().region(r.child(*it));
		}
		else {
			throw std::runtime_error("DumpGhPath: invalid path");
		}
		std::cout << "Region: " << std::endl;
		std::cout << "ghId=" << r.ghId() << std::endl;
		std::cout << "storeId=" << r.storeId() << std::endl;
		std::cout << "children.size=" << r.childrenSize() << std::endl;
		std::cout << "parents.size=" << r.parentsSize() << std::endl;
		std::cout << "cellIndex.size=" << completer.indexStore().at(r.cellIndexPtr()).size() << std::endl;
	}
}

void Worker::dumpKeyStringTable(WD_DumpKeyStringTable & d) {
	std::ofstream dksOut;
	dksOut.open(d.value.c_str());
	if (dksOut.is_open()) {
		for(auto x : completer.store().keyStringTable()) {
			dksOut << x << std::endl;
		}
		dksOut.close();
	}
}

void Worker::dumpValueStringTable(WD_DumpValueStringTable& d) {
	std::ofstream dvsOut;
	dvsOut.open(d.value.c_str());
	if (dvsOut.is_open()) {
		for(auto x : completer.store().valueStringTable()) {
			dvsOut << x << std::endl;
		}
		dvsOut.close();
	}
}

void Worker::dumpItemTags(WD_DumpItemTags& d) {
	std::ofstream ditOut;
	ditOut.open(d.value.c_str());
	if (ditOut.is_open()) {
		const liboscar::Static::OsmKeyValueObjectStore & store = completer.store();
		liboscar::Static::OsmKeyValueObjectStore::Item item;
		std::string tag;
		sserialize::ProgressInfo pinfo;
		pinfo.begin(store.size(), "Dumping item tags");
		for(uint32_t i(0), s(store.size()); i < s; ++i) {
			item = store.at(i);
			for(uint32_t j(0), js(item.size()); j < js; ++j) {
				tag = item.key(j) + ":" + item.value(j);
				std::replace(tag.begin(), tag.end(), '\n', '_');
				ditOut << tag << '\n';
			}
			pinfo(i);
		}
		pinfo.end();
		ditOut.close();
	}
}

void Worker::interactivePartial(WD_InteractivePartial & d) {
	LiveCompletion liveCompleter(completer);
	liveCompleter.doPartialCompletion(d.seek);
}

void Worker::interactiveSimple(WD_InteractiveSimple & d) {
	LiveCompletion liveCompleter(completer);
	liveCompleter.doSimpleCompletion(d.maxResultSetSize, d.minStrLen, d.printNumResults);
}

void Worker::interactiveFull(WD_InteractiveFull& d) {
	LiveCompletion liveCompleter(completer);
	liveCompleter.doFullCompletion(d.value);
}

inline std::string escapeString(const std::set<char> & escapeChars, const std::string & istr) {
	std::string ostr = "";
	for(std::string::const_iterator it = istr.begin(); it != istr.end(); ++it) {
		if (escapeChars.count(*it) > 0) {
			ostr += '\\';
		}
		ostr += *it;
	}
	return ostr;
}


template<typename T_OUTPUT_ITERATOR>
void createCompletionStrings2(const liboscar::Static::OsmKeyValueObjectStore & db, sserialize::StringCompleter::QuerryType qt, uint32_t count, T_OUTPUT_ITERATOR out) {
	std::string escapeStr = "-+/\\^$[]() ";
	sserialize::AsciiCharEscaper escaper(escapeStr.begin(), escapeStr.end());

	if (qt & sserialize::StringCompleter::QT_SUBSTRING) {
		for(size_t i = 0; i < count; i++) {
			uint32_t itemId = ((double)rand()/RAND_MAX)*db.size();
			liboscar::Static::OsmKeyValueObjectStore::Item item( db.at(itemId) );
			std::string str = "";
			for(size_t j = 0; j < item.strCount(); j++) {
				std::string s = item.strAt(j);
				if (s.size()) {
					if (*(s.begin()) == '\"') {
						s.erase(0,1);
						if (s.size() > 0 && *(s.rbegin()) == '\"') {
							s.erase(s.size()-1, 1);
						}
					}
					str += escaper.escape(s) + " ";
				}
			}
			if (qt & sserialize::StringCompleter::QT_CASE_INSENSITIVE) {
				str = sserialize::unicode_to_lower(str);
			}
			if (str.size() > 3) {
				*out = str;
				++out;
			}
		}
	}
}

void Worker::createCompletionStrings(WD_CreateCompletionStrings & d) {
	if (d.outFileName.empty()) {
		throw std::runtime_error("Create completion strings: out file not specified");
	}
	std::ofstream of;
	of.open(d.outFileName);
	if (!of.is_open()) {
		throw std::runtime_error("Create completion strings: unable to open outfile=" + d.outFileName);
	}
	std::unordered_set<std::string> result;
	sserialize::StringCompleter::QuerryType qt = (sserialize::StringCompleter::QuerryType) (sserialize::StringCompleter::QT_SUBSTRING | sserialize::StringCompleter::QT_CASE_INSENSITIVE);
	std::insert_iterator< std::unordered_set<std::string> > resInsertIt(result, result.end());
	createCompletionStrings2(completer.store(), qt, d.count, resInsertIt);
	
	for(const auto & cmpStr : result) {
		if (d.simulate) {
			std::string s = "";
			for(auto cmpStrChar : cmpStr) {
				s += cmpStrChar;
				of << s << std::endl;
			}
		}
		else {
			of << cmpStr << std::endl;
		}
	}
}

template<typename T_OUTPUT_ITERATOR>
void readCompletionStringsFromFile(const std::string & fileName, T_OUTPUT_ITERATOR out) {
	std::string tmp;
	std::ifstream inFile;
	inFile.open(fileName);
	if (!inFile.is_open()) {
		throw std::runtime_error("Failed to read completion strings from " + fileName);
	}
	while (!inFile.eof()) {
		std::getline(inFile, tmp);
		*out = tmp;
		++out;
	}
	inFile.close();
}


void Worker::completeStringPartial(WD_CompleteStringPartial & d) {
	std::vector<std::string> strs(1, d.str);
	LiveCompletion liveCompleter(completer);
	liveCompleter.doPartialCompletion(strs, d.seek, d.printNumResults);
}

void Worker::completeStringSimple(WD_CompleteStringSimple& d) {
	std::vector<std::string> strs(1, d.str);
	LiveCompletion liveCompleter(completer);
	liveCompleter.doSimpleCompletion(strs, d.maxResultSetSize, d.minStrLen, d.printNumResults);
}

void Worker::completeStringFull(WD_CompleteStringFull& d) {
	std::vector<std::string> strs(1, d.str);
	LiveCompletion liveCompleter(completer);
	liveCompleter.doFullCompletion(strs, d.printNumResults);
}

void Worker::completeStringClustered(WD_CompleteStringClustered & d, bool treedCQR) {
	std::vector<std::string> strs(1, d.str);
	LiveCompletion liveCompleter(completer);
	liveCompleter.doClusteredComplete(strs, d.printNumResults, treedCQR);
}


void Worker::completeFromFilePartial(WD_CompleteFromFilePartial & d) {
	std::vector<std::string> strs;
	readCompletionStringsFromFile(d.fileName, std::back_insert_iterator< std::vector<std::string> >(strs));
	LiveCompletion liveCompleter(completer);
	liveCompleter.doPartialCompletion(strs, d.seek, d.printNumResults);
}

void Worker::completeFromFileSimple(WD_CompleteFromFileSimple& d) {
	std::vector<std::string> strs;
	readCompletionStringsFromFile(d.fileName, std::back_insert_iterator< std::vector<std::string> >(strs));
	LiveCompletion liveCompleter(completer);
	liveCompleter.doSimpleCompletion(strs, d.maxResultSetSize, d.minStrLen, d.printNumResults);
}

void Worker::completeFromFileFull(WD_CompleteFromFileFull& d) {
	std::vector<std::string> strs;
	readCompletionStringsFromFile(d.fileName, std::back_insert_iterator< std::vector<std::string> >(strs));
	LiveCompletion liveCompleter(completer);
	liveCompleter.doFullCompletion(strs, d.printNumResults);
}

void Worker::completeFromFileClustered(WD_CompleteFromFileClustered & d, bool treedCQR) {
	std::vector<std::string> strs;
	readCompletionStringsFromFile(d.fileName, std::back_insert_iterator< std::vector<std::string> >(strs));
	LiveCompletion liveCompleter(completer);
	liveCompleter.doClusteredComplete(strs, d.printNumResults, treedCQR);
}

void Worker::symDiffCompleters(WD_SymDiffItemsCompleters& d) {
	LiveCompletion lc(completer);
	lc.symDiffItemsCompleters(d.str, d.completer1, d.completer2,d.printNumResults);
}

void Worker::benchmark(WD_Benchmark & d) {
	Benchmarker bm(completer);
	bm.benchmark(d.config);
}

}//end namespace
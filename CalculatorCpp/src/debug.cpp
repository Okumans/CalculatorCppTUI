#include <iostream>
#include <vector>
#include <unordered_set>
#include <unordered_map>

#ifdef DEBUG
template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& vec) {
	os << "[";
	for (auto it{ vec.begin() }; it != vec.end(); it++)
		os << *it << ((it + 1) != vec.end() ? ", " : "");
	os << "]";
	return os;
}

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::unordered_set<T>& uset) {
	os << "{";
	for (auto it{ uset.begin() }; it != uset.end(); it++)
		os << *it << (std::next(it) != uset.end() ? ", " : "");
	os << "}";
	return os;
}

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::unordered_map<T1, T2>& umap) {
	os << "{";
	for (auto it{ umap.begin() }; it != umap.end(); it++)
		os << it->first << ": " << it->second << (std::next(it) != umap.end() ? ", " : "");
	os << "}";
	return os;
}

#endif // DEBUG
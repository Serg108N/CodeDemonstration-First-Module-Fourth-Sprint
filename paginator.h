#pragma once

#include <vector>
#include <utility>
#include <iterator>

template<typename Iterator>
class Paginator {

public:

	Paginator(Iterator range_begin, Iterator range_end, std::size_t page_size) {
		size_ = distance(range_begin, range_end);

		Iterator it = range_begin;
		while (it != range_end) {
			if (distance(it, range_end) < page_size) {
				pages_.push_back(std::make_pair(it, range_end));
				it = range_end;
			}
			else {
				Iterator temp = it;
				advance(it, page_size);
				pages_.push_back({ temp, it });
			}
		}
	}

	auto begin() const noexcept { return pages_.begin(); }

	auto end() const noexcept { return pages_.end(); }

private:

	std::vector<std::pair<Iterator, Iterator>> pages_;
	int size_;

};

template<typename Container>
auto Paginate(const Container& c, std::size_t page_size) { return Paginator(begin(c), end(c), page_size); }

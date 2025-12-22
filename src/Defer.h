#pragma once
#include <utility>

namespace utils {

	template <typename F>
	struct ScopeGuard {
		F func;
		ScopeGuard(F&& f) : func(std::forward<F>(f)) {}
		~ScopeGuard() { func(); }
	};

	template <typename F>
	ScopeGuard(F) -> ScopeGuard<F>;

}

#define CONCAT_INTERNAL(a, b) a ## b
#define CONCAT(a, b) CONCAT_INTERNAL(a, b)

#define defer const utils::ScopeGuard CONCAT(defer_, __LINE__) = [&]

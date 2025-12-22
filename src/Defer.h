#pragma once

#include <utility>

template<typename F>
struct ScopeGuard {
	F func;
	ScopeGuard(F&& f) : func(std::forward<F>(f)) {}
	~ScopeGuard() { func(); }
};

#define CONCAT_INNER(a, b) a ## b
#define CONCAT(a, b) CONCAT_INNER(a, b)

#define defer ScopeGuard CONCAT(defer_, __LINE__) = [&]

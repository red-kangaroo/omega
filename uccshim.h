// Omega is free software, distributed under the MIT license

#pragma once
#define __STDC_LIMIT_MACROS	// Global macros turning on library features
#include <stdint.h>		// Standard includes
#include <limits.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <array>		// C++ STL includes
#include <vector>
#include <limits>
#include <numeric>
#include <algorithm>
using namespace std;

//{{{ gcc attribute macros ------------------------------------------------

#define compile_constant(x)	__builtin_constant_p(x)
#define PRINTFARGS(fmt,args)	__attribute__((__format__(__printf__,fmt,args)))
#define SCANFARGS(fmt,args)	__attribute__((__format__(__scanf__,fmt,args)))
#define NOINLINE		__attribute__((noinline))
#define NORETURN		__attribute__((noreturn))
#define CONST			__attribute__((const))
#define PURE			__attribute__((pure))
#define WEAKSYM			__attribute__((weak))
#define WEAKALIAS(sym)		__attribute__((weak,alias(sym)))
#define NONNULL(...)		__attribute__((nonnull(__VA_ARGS__)))
#define UNUSED			__attribute__((unused))
#define MALLOCLIKE		__attribute__((malloc))
#define SIMDSIZE(x)		__attribute__((vector_size(x)))
#define asm			__asm__
#if __i386__ || __x86_64__
    #define __x86__ 1
#endif

//}}}----------------------------------------------------------------------
//{{{ Array macros

/// Returns the number of elements in a static vector
template <typename T, size_t N> constexpr inline size_t ArraySize (T(&a)[N])
{
    static_assert (sizeof(a), "C++ forbids zero-size arrays");
    return N;
}
/// Returns the end() for a static vector
template <typename T, size_t N> constexpr inline T* ArrayEnd (T(&a)[N])
{ return &a[ArraySize(a)]; }
/// Expands into a ptr,size expression for the given static vector; useful as link arguments.
#define ArrayBlock(v)	&(v)[0], ArraySize(v)
/// Expands into a begin,end expression for the given static vector; useful for algorithm arguments.
#define ArrayRange(v)	&(v)[0], ArrayEnd(v)

/// Returns the number of bits in the given type
#define BitsInType(v)	(sizeof(v) * CHAR_BIT)
/// Returns the value of the given bit
template <typename T> constexpr inline bool GetBit (T v, int i) { return v&(T(1)<<i); }
/// Sets the value of the given bit
template <typename T> inline void SetBit (T& v, int i, bool b=true) { T m(T(1)<<i); T on(v|m),off(v&~m); v=b?on:off; }

/// Argument that is used only in debug builds (as in an assert)
#ifndef NDEBUG
    #define DebugArg(x)	x
#else
    #define DebugArg(x)
#endif

/// Shorthand for container iteration.
#define foreach(i,ctr)	for (auto i = (ctr).begin(); i < (ctr).end(); ++i)
/// Shorthand for container reverse iteration.
#define eachfor(i,ctr)	for (auto i = (ctr).end(); i-- > (ctr).begin();)

template <typename T> void itzero (T* v) { memset (v, 0, sizeof(T)); }

//}}}----------------------------------------------------------------------
//{{{ size_cast and sized_type

/// Selects a type by its size
template <size_t N> struct sized_type { using type = void; using stype = void; };
template <> struct sized_type<1> { using type = uint8_t ; using stype = int8_t ; };
template <> struct sized_type<2> { using type = uint16_t; using stype = int16_t; };
template <> struct sized_type<4> { using type = uint32_t; using stype = int32_t; };
template <> struct sized_type<8> { using type = uint64_t; using stype = int64_t; };

/// Casts the given value to type R with no conversion
template <typename R, typename T> constexpr const R& const_raw_cast (const T& v) { return *reinterpret_cast<const R*>(&v); }
template <typename R, typename T> constexpr R& raw_cast (T& v) { return *reinterpret_cast<R*>(&v); }

/// Casts the given value to a standard type of the same size with no conversion
template <typename T> constexpr typename sized_type<sizeof(T)>::type
    size_cast (T& v) { return const_raw_cast<typename sized_type<sizeof(T)>::type>(v); }

/// The weakest possible cast to an already convertible type
template <typename T> constexpr const T& implicit_cast (const T& v) { return v; }
template <typename T> constexpr T& implicit_cast (T& v) { return v; }

//}}}----------------------------------------------------------------------
//{{{ Arithmetic algos

namespace { // The compiler issues a warning if an unsigned type is compared to 0.
template <typename T, bool IsSigned> struct __is_negative { inline constexpr bool operator()(const T& v) { return v < 0; } };
template <typename T> struct __is_negative<T,false> { inline constexpr bool operator()(const T&) { return false; } };
}

/// Warning-free way to check if \p v is negative, even if for unsigned types.
template <typename T>
inline constexpr bool is_negative (const T& v)
    { return __is_negative<T,numeric_limits<T>::is_signed>()(v); }

/// \brief Returns the absolute value of \p v
/// Unlike the stdlib functions, this is inline and works with all types.
template <typename T>
inline constexpr T absv (T v)
    { return is_negative(v) ? -v : v; }

/// \brief Returns -1 for negative values, 1 for positive, and 0 for 0
template <typename T>
inline constexpr T sign (T v)
    { return (0 < v) - is_negative(v); }

/// The alignment performed by default.
enum { c_DefaultAlignment = alignment_of<void*>::value };

/// \brief Rounds \p n down to be divisible by \p grain
template <typename T>
inline constexpr T Floor (T n, size_t grain = c_DefaultAlignment)
    { return n - n % grain; }

/// \brief Rounds \p n up to be divisible by \p grain
template <typename T>
inline constexpr T Align (T n, size_t grain = c_DefaultAlignment)
    { return Floor<T> (n+grain-1, grain); }

/// \brief Divides \p n1 by \p n2 and rounds the result up.
/// This is in contrast to regular division, which rounds down.
template <typename T1, typename T2>
inline constexpr T1 DivRU (T1 n1, T2 n2)
    { return (n1 + (is_negative(n1) ? -(n2-1) : n2-1)) / n2; }

inline constexpr uint32_t vpack (uint8_t a, uint8_t b, uint8_t c, uint8_t d)
    { return (d<<24)|(c<<16)|(b<<8)|a; }
inline constexpr uint32_t vpack (uint8_t a, uint8_t b, uint16_t c)
    { return (c<<16)|(b<<8)|a; }

inline void srandrand (void)	{ srand (getpid()^time(nullptr)); }
unsigned xrand (void);
void sxrand (unsigned seed);

//}}}----------------------------------------------------------------------
//{{{ Container algos

#define iat(i)	begin()+i

template <typename Ctr>
inline typename Ctr::iterator p2i (Ctr& ctr, typename Ctr::pointer p)
    { return ctr.begin()+(p-&*ctr.begin()); }

template <typename Ctr>
inline void fill (Ctr& ctr, typename Ctr::const_reference v)
    { fill_n (ctr.begin(), ctr.size(), v); }

template <typename T> inline void movsv (const T*& s, size_t n, T*& d)
{
#if __x86__
	 if (sizeof(T) == 1) asm volatile ("rep\tmovsb":"+S"(s),"+D"(d),"+c"(n)::"memory");
    else if (sizeof(T) == 2) asm volatile ("rep\tmovsw":"+S"(s),"+D"(d),"+c"(n)::"memory");
    else if (sizeof(T) == 4) asm volatile ("rep\tmovsl":"+S"(s),"+D"(d),"+c"(n)::"memory");
#if __x86_64__
    else if (sizeof(T) == 8) asm volatile ("rep\tmovsq":"+S"(s),"+D"(d),"+c"(n)::"memory");
#endif
    else
#endif
    while (n--) *d++ = *s++;
}

template <typename T> static inline unsigned scasv (const T*& p, unsigned n, T v = T())
{
#if __x86__
	 if (sizeof(T) == 1) asm volatile ("repnz\tscasb":"+D"(p),"+c"(n):"a"(v):"memory");
    else if (sizeof(T) == 2) asm volatile ("repnz\tscasw":"+D"(p),"+c"(n):"a"(v):"memory");
    else if (sizeof(T) == 4) asm volatile ("repnz\tscasl":"+D"(p),"+c"(n):"a"(v):"memory");
#if __x86_64__
    else if (sizeof(T) == 8) asm volatile ("repnz\tscasq":"+D"(p),"+c"(n):"a"(v):"memory");
#endif
    else
#endif
    do { ++p; } while (--n && p[-1] != v);
    return n;
}

static inline const char* strnext_r (const char* s, unsigned& n)
{
#if __x86__
    if (!compile_constant(strlen(s)))
	n = scasv (s, n, '\0');
    else
#endif
	s+=strlen(s)+1;
    return s;
}
static inline const char* strnext (const char* s) PURE;
static inline const char* strnext (const char* s) { unsigned n = -1; return strnext(s,n); }

//}}}----------------------------------------------------------------------
//{{{ Curses shims

#define KEY_ESCAPE	0x1b
struct KEY_CTRL_MOD {	inline constexpr int operator| (char c) { return c-'a'+1; } };
#define KEY_CTRL	KEY_CTRL_MOD()

#define COLOR_DEFAULT	-1

//}}}----------------------------------------------------------------------
//{{{ memlink and memblock

class memlink {
public:
    using value_type		= uint8_t;
    using pointer		= value_type*;
    using const_pointer		= const value_type*;
    using iterator		= pointer;
    using const_iterator	= const_pointer;
    using size_type		= uint32_t;
    using difference_type	= int32_t;
public:
    inline constexpr		memlink (void)		: _data (nullptr), _size(0) {}
    inline			memlink (pointer p, size_type sz)	: _data(p), _size(sz) {}
    inline void			resize (size_type sz)	{ _size = sz; }
    inline void			link (pointer p, size_type sz)	{ _data = p; resize(sz); }
    inline size_type		size (void) const	{ return _size; }
    inline iterator		begin (void)		{ return _data; }
    inline const_iterator	begin (void) const	{ return _data; }
    inline iterator		end (void)		{ return begin()+size(); }
    inline const_iterator	end (void) const	{ return begin()+size(); }
    inline void			swap (memlink&& b)	{ ::std::swap(_data,b._data); ::std::swap(_size,b._size); }
private:
    pointer			_data;
    size_type			_size;
};

using cmemlink = memlink;

class memblock : public memlink {
public:
    inline constexpr		memblock (void)		: memlink() {}
    inline explicit		memblock (size_type sz) : memlink ((pointer) malloc (sz), sz) {}
    inline			~memblock (void)	{ if (begin()) free(begin()); }
    inline void			resize (size_type sz)	{ link ((pointer) realloc (begin(), sz), sz); }
    inline void			swap (memblock&& b)	{ memlink::swap (move(b)); }
    void			read_file (const char* filename);
    void			write_file (const char* filename);
};

//}}}----------------------------------------------------------------------
//{{{ Serialization streams

//{{{2 bstrb --------------------------------------------------------------

class bstrb {
public:
    using value_type	= unsigned char;
    using size_type	= unsigned int;
    using pointer	= value_type*;
    using const_pointer	= const value_type*;
    enum { is_sizing = false, is_reading = false, is_writing = false };
protected:
    inline constexpr size_type	align_size (size_type sz, size_type g) const	{ return (g-1)-((sz+(g-1))%g); }
    inline constexpr size_type	align_size (const_pointer p, size_type g) const	{ return align_size(p-(pointer)nullptr,g); }
};

//}}}2---------------------------------------------------------------------
//{{{2 bstrs

class bstrs : public bstrb {
public:
    enum { is_sizing = true };
private:
    inline size_type	wrstrlen (const char* s) const	{ return s?strlen(s):0; }
    template <typename T, bool pod> struct type_writer { static inline void write (bstrs& os, const T& v) { v.write (os); } };
    template <typename T> struct type_writer<T,true> { static inline void write (bstrs& os, const T& v) { os.iwrite (v); } };
public:
    inline explicit	bstrs (size_type sz =0)	:_sz(sz) { }
    template <typename T>
    inline T*		iptr (void)		{ return nullptr; }
    inline size_type	remaining (void) const	{ return UINT_MAX; }
    inline size_type	pos (void) const	{ return _sz; }
    inline size_type	size (void) const	{ return pos(); }
    inline void		skip (int n)		{ _sz += n; }
    inline void		align (size_type g)	{ skip (align_size(size(),g)); }
    inline void		skipalign (size_type g)	{ align (g); }
    inline void		write (const void*, size_type sz)	{ skip(sz); }
    inline void		write_strz (const char* v)		{ skip(wrstrlen(v)+1); }
    template <typename T>
    inline void		iwrite (const T& v)			{ write (&v, sizeof(v)); }
    template <typename T>
    inline void		uiwrite (const T& v)			{ write (&v, sizeof(v)); }
    template <typename T>
    inline bstrs&	operator<< (const T& v)			{ type_writer<T,is_pod<T>::value>::write (*this, v); return *this; }
    inline bstrs&	operator<< (const char* s);
    inline bstrs&	operator<< (const string& s)		{ iwrite(uint32_t(s.size()+1)); write(s.c_str(),s.size()+1); align(4); return *this; }
    template <typename T, size_t N>
    inline bstrs&	operator<< (const array<T,N>& v)	{ foreach (i,v) *this << *i; return *this; }
    template <typename T>
    inline bstrs&	operator<< (const vector<T>& v)		{ iwrite(uint32_t(v.size())); foreach (i,v) *this << *i; align(4); return *this; }
    template <typename T>
    inline bstrs&	operator& (const T& v)			{ return *this << v; }
private:
    size_type		_sz;
};

//}}}2---------------------------------------------------------------------
//{{{2 bstro

class bstro : public bstrb {
public:
    enum { is_writing = true };
private:
    template <typename T, bool pod> struct type_writer { static inline void write (bstro& os, const T& v) { v.write (os); } };
    template <typename T> struct type_writer<T,true> { static inline void write (bstro& os, const T& v) { os.iwrite (v); } };
public:
    inline		bstro (pointer p, size_type sz)	:_p(p),_pend(_p+sz),_sz(sz) {}
    inline		bstro (memblock& b)	: _p(b.begin()),_pend(b.end()),_sz(b.size()) {}
    inline size_type	pos (void) const	{ return _sz-(_pend-_p); }
    inline pointer	ipos (void)		{ return _p; }
   inline const_pointer	ipos (void) const	{ return _p; }
   inline const_pointer	end (void) const	{ return _pend; }
    template <typename T>
    inline T*		iptr (void)		{ return reinterpret_cast<T*>(ipos()); }
    inline size_type	remaining (void) const	{ return end()-ipos(); }
    inline size_type	size (void) const	{ return remaining(); }
    inline void		iseek (pointer p)	{ assert(p <= _pend && "stream overflow"); _p = p; }
    inline void		skip (int n)		{ iseek(_p+n); }
    inline void		align (size_type g)	{ const size_type nz = align_size(ipos(),g); memset(ipos(),0,nz); skip(nz); }
    inline void		skipalign (size_type g)	{ skip(align_size(ipos(),g)); }
    template <typename T>
    inline void		iwrite (const T& v)	{ *iptr<T>() = v; skip(sizeof(v)); }
    template <typename T>
    inline void		uiwrite (const T& v);
    inline void		write (const void* v, size_type sz)	{ pointer o = _p; skip(sz); memcpy (o,v,sz); }
    inline void		write_strz (const char* v)		{ write (v, strlen(v)+1); }
    template <typename T>
    inline bstro&	operator<< (const T& v)		{ type_writer<T,is_pod<T>::value>::write (*this, v); return *this; }
    inline bstro&	operator<< (const char* s);
    inline bstro&	operator<< (const string& s)	{ iwrite(uint32_t(s.size()+1)); write(s.c_str(),s.size()+1); align(4); return *this; }
    template <typename T, size_t N>
    inline bstro&	operator<< (const array<T,N>& v){ foreach (i,v) *this << *i; return *this; }
    template <typename T>
    inline bstro&	operator<< (const vector<T>& v)	{ iwrite(uint32_t(v.size())); foreach (i,v) *this << *i; align(4); return *this; }
    template <typename T>
    inline bstro&	operator& (const T& v)		{ return *this << v; }
private:
    pointer		_p;
    const_pointer	_pend;
    size_type		_sz;
};

//}}}2---------------------------------------------------------------------
//{{{2 bstri

class bstri : public bstrb {
public:
    enum { is_reading = true };
private:
    template <typename T, bool pod> struct type_reader { static inline void read (bstri& os, T& v) { v.read (os); } };
    template <typename T> struct type_reader<T,true> { static inline void read (bstri& os, T& v) { os.iread (v); } };
public:
    inline		bstri (const_pointer p, size_type sz)	:_p(p),_pend(_p+sz) {}
    inline explicit	bstri (const memblock& b)		:_p(b.begin()),_pend(b.end()) {}
    inline void		link (const_pointer p, size_type sz)	{ _p = p; _pend = p+sz; }
    inline void		link (const memblock& b)		{ link (b.begin(), b.size()); }
   inline const_pointer	ipos (void) const	{ return _p; }
   inline const_pointer	end (void) const	{ return _pend; }
    template <typename T>
    inline const T*	iptr (void) const	{ return reinterpret_cast<const T*>(ipos()); }
    inline size_type	remaining (void) const	{ return end()-ipos(); }
    inline size_type	size (void) const	{ return remaining(); }
    inline void		iseek (const_pointer i)	{ assert(i <= end() && "stream underflow"); _p = i; }
    inline void		skip (int n)		{ iseek (ipos()+n); }
    inline void		align (size_type g)	{ skip (align_size(ipos(),g)); }
    inline void		skipalign (size_type g)	{ align (g); }
    void		verify_remaining (const char* f, size_type sz) const;
    inline void		read (void* v, size_type sz)	{ assert(remaining()>=sz && "read overflow"); memcpy (v,ipos(),sz); skip(sz); }
    inline const char*	read_strz (void)		{ const char* v = iptr<char>(); skip(strlen(v)+1); return ipos() <= end() ? v : nullptr; }
    template <typename T>
    inline void		iread (T& v)			{ v = *iptr<T>(); skip(sizeof(v)); }
    template <typename T>
    inline void		uiread (T& v);
    inline bstri&	operator>> (string& s)		{ uint32_t sz; iread(sz); s.assign (iptr<char>(), sz-1); skip(sz); align(4); return *this; }
    template <typename T, size_t N>
    inline bstri&	operator>> (array<T,N>& v)	{ foreach (i,v) *this >> *i; return *this; }
    template <typename T>
    inline bstri&	operator>> (vector<T>& v)	{ uint32_t sz; iread(sz); v.resize(sz); foreach (i,v) *this >> *i; align(4); return *this; }
    template <typename T>
    inline bstri&	operator>> (T& v)		{ type_reader<T,is_pod<T>::value>::read (*this, v); return *this; }
    inline bstri&	operator>> (const char*& s);
    template <typename T>
    inline bstri&	operator& (T& v)		{ return *this >> v; }
private:
    const_pointer	_p;
    const_pointer	_pend;
};

//}}}2---------------------------------------------------------------------
//{{{2 inline bodies

template <typename T>
inline constexpr streamsize stream_align_of (const T& v) { return alignof(v); }
#define STREAM_ALIGN(type,grain)	\
template <> inline constexpr streamsize stream_align_of (const type&) { return grain; }
STREAM_ALIGN(string,4)
template <typename T>
inline constexpr streamsize stream_align_of (const vector<T>& v) { return 4; }
template <typename T, size_t N>
inline constexpr streamsize stream_align_of (const array<T,N>& v) { return stream_align_of(v[0]); }

template <typename T>
streamsize stream_size_of (const T& v)	{ bstrs ss; ss << v; return ss.pos(); }

inline bstrs& bstrs::operator<< (const char* s)
{
    skip (sizeof(size_type)+Align(wrstrlen(s)+1,4));
    return *this;
}

template <typename T>
inline void bstro::uiwrite (const T& v)
{
    #if __x86__
	iwrite (v);
    #else
	write (&v, sizeof(v));
    #endif
}

inline bstro& bstro::operator<< (const char* s)
{
    size_type sl = strlen(s)+1;
    operator<< (sl);
    write (s,sl);
    align (4);
    return *this;
}

template <typename T>
inline void bstri::uiread (T& v)
{
    #if __x86__
	iread (v);
    #else
	read (&v, sizeof(v));
    #endif
}

inline bstri& bstri::operator>> (const char*& s)
{
    size_type sl;
    operator>> (sl);
    sl += align_size (sl,4);
    s = iptr<char>();
    if (sl > remaining())
	s = nullptr;
    else
	skip (sl);
    return *this;
}

//}}}2---------------------------------------------------------------------
//}}}----------------------------------------------------------------------

#ifndef JSON_WRITER_H
#define JSON_WRITER_H

#include <cstdint>
#include <cstring>
#include <iomanip>
#include <ostream>
#include <type_traits>


class json_writer {
public:
	json_writer(std::ostream& sout, int indent = 0);

	// object start / end
	void array_start();
	void array_end();
	void object_start();
	void object_end();

	// write name
	void name(const char* first, const char* last);
	void name(const char* strz);

	// write value
	void value(const char* first, const char* last);
	void value(const char* strz);
	void value_bool(bool b);
	void value_null();

	// numbers
	template<class T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
	inline void value(T n) {
		value_sep();
		sout_ << n;
		sep_status_ = SepStatus::next;
	}

	// string support
	template <class StringT, typename = typename std::enable_if<std::is_class<StringT>::value>::type>
	void name(const StringT& str) {
		name(str.data(), str.data() + str.size());
	}
	template <class StringT, typename = typename std::enable_if<std::is_class<StringT>::value>::type>
	void value(const StringT& str) {
		value(str.data(), str.data() + str.size());
	}

protected:
	void value_sep();
	void indent();

	void write_string(const char* first, const char* last);
	
private:
	enum class SepStatus {
		none,
		first,
		next
	};
	std::ostream& sout_;
	const int indent_;
	int level_;
	SepStatus sep_status_;
};

// json_writer inline

namespace {

static const char cObjStart = '{';
static const char cObjEnd   = '}';
static const char cArrStart = '[';
static const char cArrEnd   = ']';
static const char sNameSep[]  = ": ";
static const char cValueSep = ',';

}

inline json_writer::json_writer(std::ostream& sout, int indent)
	: sout_(sout)
	, indent_(indent)
	, level_(0)
	, sep_status_(SepStatus::none)
{}

// object start / end

inline void json_writer::array_start() {
	value_sep();
	sout_ << cArrStart;
	sep_status_ = SepStatus::first;
	level_++;
}

inline void json_writer::array_end() {
	level_--;
	indent();
	sout_ << cArrEnd;
	sep_status_ = SepStatus::next;
}

inline void json_writer::object_start() {
	value_sep();
	sout_ << cObjStart;
	sep_status_ = SepStatus::first;
	level_++;
}

inline void json_writer::object_end() {
	level_--;
	indent();
	sout_ << cObjEnd;
	sep_status_ = SepStatus::next;
}

// write name

inline void json_writer::name(const char* first, const char* last) {
	value_sep();
	write_string(first, last);
	sout_.write(sNameSep, indent_ ? 2 : 1);
	sep_status_ = SepStatus::none;
}

inline void json_writer::name(const char* strz) {
	name(strz, strz + std::strlen(strz));	
}

// write value

inline void json_writer::value(const char* first, const char* last) {
	value_sep();
	write_string(first, last);
	sep_status_ = SepStatus::next;
}

inline void json_writer::value(const char* strz) {
	value(strz, strz + std::strlen(strz));	
}

inline void json_writer::value_bool(bool b) {
	value_sep();
	sout_ << (b ? "true" : "false");
	sep_status_ = SepStatus::next;
}

inline void json_writer::value_null() {
	value_sep();
	sout_ << "null";
	sep_status_ = SepStatus::next;
}

// write value separator

inline void json_writer::value_sep() {
	if (sep_status_ == SepStatus::next)
		sout_ << cValueSep;
	indent();
}

// EOL and indent

inline void json_writer::indent() {
	if (indent_ && sep_status_ != SepStatus::none)
		sout_ << '\n' << std::setw(level_ * indent_) << "";
}

// write string

namespace {

static const char cQuote        = '\"';
static const char cBackslash    = '\\';
static const char sUnicodeEsc[] = { '\\', 'u', '0', '0' };

static inline char num_to_hex(int n) {
	static const char sHex[] = { '0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };
	return sHex[n];
}

}

inline void json_writer::write_string(const char* first, const char* last)
{
	sout_ << cQuote;

	const char* start = first;
	for (const char* ps = first; ps != last; ps++) {
		const unsigned char c = static_cast<unsigned char>(*ps);
		
		if (c == cQuote || c == cBackslash) {	
			sout_.write(start, ps - start);
			sout_ << cBackslash;
			sout_ << c;
			start = ps + 1;
		} else if (c <= 31 || c == 127) {
			sout_.write(start, ps - start);
			switch (c) {
			case '\b':
				sout_ << cBackslash;
				sout_ << (char)'b';
				break;
			case '\f':
				sout_ << cBackslash;
				sout_ << (char)'f';
				break;
			case '\n':
				sout_ << cBackslash;
				sout_ << (char)'n';
				break;
			case '\r':
				sout_ << cBackslash;
				sout_ << (char)'r';
				break;
			case '\t':
				sout_ << cBackslash;
				sout_ << (char)'t';
				break;
			default:
				sout_.write(sUnicodeEsc, 4);
				sout_ << num_to_hex((c >> 4) & 0xF);
				sout_ << num_to_hex(c & 0xF);
				break;
			}
			start = ps + 1;
		}
	}
	if (start != last)
		sout_.write(start, last - start);

	sout_ << cQuote;
}

#endif // JSON_WRITER_H

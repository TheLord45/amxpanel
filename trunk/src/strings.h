/*
 *   Copyright (C) 2018 by Andreas Theofilu (TheoSys) <andreas@theosys.at>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation version 3 of the License.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __STRINGS_H__
#define __STRINGS_H__

#include <string>
#include <vector>
#include <iostream>
#include <cstring>
#include <stdexcept>

// Which C++ version is supported?
#ifdef __SUNPRO_CC
    #define _CPP_STD 3
#elif defined(__GNUC__) && defined(__cplusplus)
    #if __cplusplus >= 201103L
        #define _CPP_STD 11
    #else
        #define _CPP_STD 3
    #endif
#elif defined(__cplusplus)
    #if __cplusplus == 199711L
        #define _CPP_STD 3
    #elif __cplusplus == 201103L
        #define _CPP_STD 11
    #else
        #define _CPP_STD 3
    #endif
#else
    #define _CPP_STD 3
#endif

// 32 or 64 bit environment?
// GCC
#if __GNUC__
    #if __x86_64__ || __ppc64__ || __amd64
        #define ENVIRONMENT64
    #else
        #define ENVIRONMENT32
    #endif
#endif
// Oracle/Sun developer studio
#ifdef __SUNPRO_CC
    #if defined(_ILP32) || defined(__i386)
        #define ENVIRONMENT32
    #elif _LP64 || __amd64
        #define ENVIRONMENT64
    #endif
#endif

/**
 * \namespace strings strings.h
 *
 * \brief
 * Contains the definations for the \a String class.
 *
 * This contains the definations needed for the class String.
 */
namespace strings
{
    #define MEM_BLOCK   8192        // Size of one memory block
    #define MAX_BLOCK   131072      // 128 Kib max.

    #define STHROW(msg) throw std::invalid_argument(msg)
    #define SCATCH(err) const std::exception& err

    #define ERR_OUT_OF_BOUNDS   "Index is out of bound"
    #define ERR_INVALID_INDEX   "Index is invalid"
    #define ERR_OUT_OF_MEMORY   "Can't allocate more than 131072 bytes of memory!"

#ifdef ENVIRONMENT32
    typedef long long           LONG;
    typedef unsigned long long  ULONG;
#else
    typedef long                LONG;
    typedef unsigned long       ULONG;
#endif

    /**
     * \enum REPLACE
     *
     * \brief
     * Specifies semantics used by replace functions.
     *
     * This allows to specify whether all, only the first or only the last
     * occurence of a string should be replaced.
     */
    enum REPLACE
    {
        ALL,    //!< Replaces all occurances in a string.
        FIRST,  //!< Replace only the first occurance in a string.
        LAST    //!< Replace only the last occurance in a string.
    };

    class String;
    class Regexp;

    extern void SThrow(const String& err);
    extern void SThrow(const std::string& err);
    extern void SThrow(const char *err);

    /**
     * \class strings::String strings.h
     *
     * \brief
     * Offers powerful and convenient facilities for manipulating strings.
     *
     * Class strings::String offers powerful and convenient facilities for
     * manipulating strings.
     *
     * \note
     * String is designed for use with single or multibyte character
     * sequences. Manipulating wide strings is not supported.
     *
     * Although the class is primarily intended to be used to handle single-byte
     * character sets (SBCS; such as US-ASCII or ISO Latin-1), with care it can
     * be used to handle multibyte character sets (MBCS). There are two things
     * that must be kept in mind when working with MBCS:
     *
     * - Because characters can be more than one byte long, the number of bytes
     * in a string can, in general, be greater than the number of characters in
     * the string. Use function String::length() to get the number of bytes
     * in a string, and function String::mbLength() to get the number of
     * characters. Note that the latter is much slower because it must determine
     * the number of bytes in every character. Hence, if the string is known to
     * use a SBCS, then String::length() is preferred.
     * - One or more bytes of a multibyte character can be zero. Hence, MBCS
     * cannot be counted on being null-terminated. In practice, it is a rare
     * MBCS that uses embedded nulls. Nevertheless, you should be aware of this
     * and program defensively. In any case, class String can handle embedded
     * nulls.
     *
     * \synopsis
     * #include <strings.h>
     *
     * strings::String a;
     * \endsynopsis
     *
     * \persistence
     *
     * \example
     * \code
     * #include <strings.h>
     *
     * int main()
     * {
     *     strings::String a("There is no joy in Beantown.");
     *
     *     std::cout << "\"" << a << "\"" << " becomes ";
     *
     *     a.replace("Beantown", "Redmond");
     *
     *     std::cout << "\"" << a << "\"" << std::endl;
     *
     *     return 0;
     * }
     * \endcode
     *
     * Program output:
     *
     * \code
     * "There is no joy in Beantown" becomes "There is no joy in Redmond."
     * \endcode
     * \endexample
     */
    class String
    {
        public:
            /**
             * Creates a string of length zero (the null string).
             */
            String() { _len = 0; _allocated = 0; _string = 0; }
            /**
             * Creates a string and initializes it with the contents of \a str.
             */
            String(const String& str);
            /**
             * Creates a string and initializes it with the contents of \a str.
             * It takes from string \a str the rest of the string starting
             * with index \a pos and optional the length \a len.
             */
            String(const String& str, size_t pos, size_t len = std::string::npos);
            /**
             * Creates a string and initializes it with \a s.
             */
            String(const char *s);
            /**
             * Creates a string and initializes it with \a s, taking only \a n
             * bytes.
             */
            String(const char *s, size_t n);
            /**
             * Creates a string and initializes it with \a str.
             */
            String(const std::string& str);
            /**
             * Creates a string of the size \a n and initializes it with the
             * character \a c.
             */
            String(size_t n, char c);
            /**
             * Creates a string and initializes it with the string
             * representation of value \a i.
             */
            String(int i);
            /**
             * Creates a string and initializes it with the string
             * representation of value \a l.
             */
            String(LONG l);
            /**
             * Creates a string and initializes it with the string
             * representation of value \a i.
             */
            String(unsigned int i);
            /**
             * Creates a string and initializes it with the string
             * representation of value \a l.
             */
            String(ULONG l);
            /**
             * Creates a string and initializes it with the string
             * representation of value \a f.
             */
            String(float f);
            /**
             * Creates a string and initializes it with the string
             * representation of value \a d.
             */
            String(double d);
			/**
			 * Takes a regular expression as argument and sets the result in
			 * \b self. Look at \a Regexp for details about regular expessions.
			 *
			 * @param re
			 * This is the class \a Regexp. The regular expression is used on
			 * \b self.
			 */
            String(const Regexp& re);
            /**
             * The destructor. This is called automatically at the moment the
             * class is destroyed. Do not call this directly!
             */
            ~String();

            /**
             * Returns the length of the string in bytes.
             *
             * @return
             * The length of the bytes.
             */
            size_t length() { return _len; }
            /**
             * Returns the length of the string in bytes.
             *
             * @return
             * The length of the bytes.
             */
            size_t length() const { return _len; }
            /**
             * Returns the size of the allocated buffer.
             *
             * @return
             * The size of the allocated memory.
             */
            size_t size() { return _allocated; }
            /**
             * Returns the size of the allocated buffer.
             *
             * @return
             * The size of the allocated memory.
             */
            size_t size() const { return _allocated; }
            /**
             * Resize the allocated memory to \a n bytes. If the content was
             * longer than \a n bytes, it will be trucated.
             *
             * @param n
             * New size of the internal buffer.
             */
            void resize(size_t n);
            /**
             * Resize the allocated memory to \a n bytes. The content of the
             * buffer will be initialized with character \a c.
             *
             * @param n
             * New size of the internal buffer.
             *
             * @param c
             * The character to initialized the buffer with.
             */
            void resize(size_t n, char c);
            /**
             * Returns the maximum size that can be allocated.
             *
             * @return
             * The maximum zize that can be allocated.
             */
            size_t max_size() { return MAX_BLOCK; }
            /**
             * Clears the internal buffer (frees the memory). All contents
             * of the buffer will be lost!
             */
            void clear();
            /**
             * Checks whether the content is empty or not.
             *
             * @return
             * TRUE if the content is empty, FALSE if it is not.
             */
            bool empty() { return (_len == 0); }
            /**
             * Checks whether the content is empty or not.
             *
             * @return
             * TRUE if the content is empty, FALSE if it is not.
             */
            bool empty() const { return (_len == 0); }
            /**
             * Returns a refernce to the last character in the string.
             *
             * @return
             * The reference to the last character in the string.
             */
            char& back() { return *(_string+_len-1); }
            /**
             * Returns a refernce to the first character in the string.
             *
             * @return
             * The reference to the first character in the string.
             */
            char& front() { return *(_string); }
            /**
             * Returns the pointer to the first character in the string.
             *
             * @return
             * The pointer to the first character in the string.
             */
            char *begin() { return _string; }
            /**
             * Returns the pointer to the first character in the string.
             *
             * @return
             * The pointer to the first character in the string.
             */
            const char *begin() const { return _string; }
            /**
             * Returns the pointer to the last character in the string.
             *
             * @return
             * The pointer to the last character in the string.
             */
            char *end() { return _string+_len-1; }
            /**
             * Returns the pointer to the last character in the string.
             *
             * @return
             * The pointer to the last character in the string.
             */
            const char *end() const { return _string+_len-1; }
            /**
             * Puts a character to the end of the string.
             *
             * @param c
             * The character <b>c</b> is appended to the end of the string.
             */
            void push_back(char c);
            /**
             * Removes the last character from the string.
             */
            void pop_back();
            /**
             * Returns the internal buffer as a <b>std::string</b> class.
             *
             * @return
             * The internal buffer as a <b>std::string</b>.
             */
            std::string toString() { std::string s(_string, _len); return s; }
            /**
             * Returns the internal buffer as a <b>std::string</b> class.
             *
             * @return
             * The internal buffer as a <b>std::string</b>.
             */
            const std::string toString() const { if (_len == 0) return ""; const std::string s(_string, _len); return s; }
            /**
             * Returns a const pointer to the internal buffer.
             *
             * @return
             * A const pointer to the internal buffer.
             */
            const char *data() { return _string; }
            /**
             * Returns a const pointer to the internal buffer.
             *
             * @return
             * A const pointer to the internal buffer.
             */
            const char *data() const { return _string; }
            /**
             * Checks whether the internal buffer is a NULL string or not.
             *
             * @return
             * TRUE if the internal buffer is a NULL string, FALSE if it is not.
             */
            bool isNull() { return (_string == 0); }
            /**
             * Converts the internal string into lower case letters.
             *
             * @return
             * A pointer to self.
             */
            String& toLower();
            /**
             * Converts the internal string into upper case letters.
             *
             * @return
             * A pointer to self.
             */
            String& toUpper();
            /**
             * Checks whether the content of the string is pure ASCII. That means
             * all characters are in the range of 0 to 127.
             *
             * @return
             * TRUE if all characters are pure ASCII, FALSE if one of them is not.
             */
            bool isAscii();
            /**
             * Checks whether the content of the string is pure ASCII. That means
             * all characters are in the range of 0 to 127.
             *
             * @return
             * TRUE if all characters are pure ASCII, FALSE if one of them is not.
             */
            bool isAscii() const;
            /**
             * Checks whether the content of the string is a numeric value. That
             * means, that all characters are in the the range of '0' to '9'. It
             * makes sure that there is only one komma present, if any and the
             * signs \b + and \b - are only allowed on the first position.
             *
             * @return
             * TRUE if all characters are numeric, FALSE if one of them is not.
             */
            bool isNumeric();
            /**
             * Checks whether the content of the string is a numeric value. That
             * means, that all characters are in the the range of '0' to '9'. It
             * makes sure that there is only one komma present, if any and the
             * signs \b + and \b - are only allowed on the first position.
             *
             * @return
             * TRUE if all characters are numeric, FALSE if one of them is not.
             */
            bool isNumeric() const;
            /**
             * Reads from a file until the whole file was read or the maximum
             * amount of memory exceeded.
             *
             * @param stream
             * A stream to an open file.
             *
             * @return
             * The stream to an open file.
             */
            std::istream& readFile(std::istream& stream);
            /**
             * Reads from a file or stream until the delimeter \a delim is found.
             *
             * @param stream
             * A stream to an open file or stream.
             *
             * @param delim
             * A delimeter as a character.
             *
             * @return
             * The stream.
             */
            std::istream& readToDelim(std::istream& stream, char delim);
            /**
             * Reads from a file or stream until the end of a line is found.
             *
             * @param stream
             * A stream to an open file or stream.
             *
             * @return
             * The stream.
             */
            std::istream& readLine(std::istream& stream);
            /**
             * Reads from a stream until the end of the string is found.
             *
             * @param stream
             * A stream to a string.
             *
             * @return
             * The stream.
             */
            std::istream& readString(std::istream& stream);

            /**
             * Removes the bytes from index \a pos, which must be no greater than
             * length(), to the end of string. Returns a reference to self.
             *
             * @param pos
             * The index to the position where the string should end.
             *
             * @return
             * A pointer to self.
             */
            String& remove(size_t pos);


            /**
             * Return the character at \a pos, which must be no greater than
             * length().
             *
             * @param pos
             * The index of the character to return.
             *
             * @return
             * The character where the index \a pos is pointing to.
             */
            inline char at(size_t pos)
            {
                if (pos < _len)
                    return *(_string+pos);

                return 0;
            }

            /**
             * Return the character at \a pos, which must be no greater than
             * length().
             *
             * @param pos
             * The index of the character to return.
             *
             * @return
             * The character where the index \a pos is pointing to.
             */
            inline char at(size_t pos) const
            {
                if (pos < _len)
                    return *(_string+pos);

                return 0;
            }

            /**
             * Assigns a string. The contents of the string are lost and instead
             * the new string will be stored.
             *
             * @param str
             * A class String containing the new string.
             *
             * @return
             * A pointer to self.
             */
            String& assign(const String& str);
            /**
             * Assigns a string. The contents of the string are lost and instead
             * the new string will be stored.
             *
             * @param str
             * A class String containing the new string.
             *
             * @param pos
             * The index of the position to \a str from where the new string
             * should be copied.
             *
             * @param len
             * Optional: The length of the string \a str to copy.
             *
             * @return
             * A pointer to self.
             */
            String& assign(const String& str, size_t pos, size_t len = std::string::npos);
            /**
             * Assigns a string. The contents of the string are lost and instead
             * the new string will be stored.
             *
             * @param s
             * A character pointer pointing to the new string.
             *
             * @return
             * A pointer to self.
             */
            String& assign(const char *s);
            /**
             * Assigns a string. The contents of the string are lost and instead
             * the new string will be stored.
             *
             * @param s
             * A character pointer pointing to the new string.
             *
             * @param n
             * The length of the string to copy.
             *
             * @return
             * A pointer to self.
             */
            String& assign(const char *s, size_t n);
            /**
             * Assigns a string. The contents of the string are lost and instead
             * the new string will be stored.
             *
             * @param str
             * A \b std::string class of the new string.
             *
             * @return
             * A pointer to self.
             */
            String& assign(const std::string& str);
            /**
             * Assigns a string. The contents of the string are lost and instead
             * the new string will be stored.
             *
             * @param i
             * An integer containing the value to convert and assign to.
             *
             * @return
             * A pointer to self.
             */
            String& assign(int i);
            /**
             * Assigns a string. The contents of the string are lost and instead
             * the new string will be stored.
             *
             * @param l
             * A long integer containing the value to convert and assign to.
             *
             * @return
             * A pointer to self.
             */
            String& assign(LONG l);
            /**
             * Assigns a string. The contents of the string are lost and instead
             * the new string will be stored.
             *
             * @param i
             * An unsigned integer containing the value to convert and assign to.
             *
             * @return
             * A pointer to self.
             */
            String& assign(unsigned int i);
            /**
             * Assigns a string. The contents of the string are lost and instead
             * the new string will be stored.
             *
             * @param l
             * An unsigned long integer containing the value to convert and assign to.
             *
             * @return
             * A pointer to self.
             */
            String& assign(ULONG l);
            /**
             * Assigns a string. The contents of the string are lost and instead
             * the new string will be stored.
             *
             * @param f
             * An float containing the value to convert and assign to.
             *
             * @return
             * A pointer to self.
             */
            String& assign(float f);
            /**
             * Assigns a string. The contents of the string are lost and instead
             * the new string will be stored.
             *
             * @param d
             * A double containing the value to convert and assign to.
             *
             * @return
             * A pointer to self.
             */
            String& assign(double d);

            /**
             * Appends a string to the end of the content.
             *
             * @param str
             * The String class to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(const String& str);
            /**
             * Appends a string to the end of the content.
             *
             * @param str
             * The String class to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(const String& str) const;
            /**
             * Appends a string to the end of the content.
             *
             * @param str
             * The String class to append.
             *
             * @param pos
             * The index to the position from where the string \a str should be copied.
             *
             * @param len
             * Optional: The length of the string \a str to copy.
             *
             * @return
             * A pointer to self.
             */
            String& append(const String& str, size_t pos, size_t len = std::string::npos);
            /**
             * Appends a string to the end of the content.
             *
             * @param str
             * The String class to append.
             *
             * @param pos
             * The index to the position from where the string \a str should be copied.
             *
             * @param len
             * Optional: The length of the string \a str to copy.
             *
             * @return
             * A pointer to self.
             */
            String& append(const String& str, size_t pos, size_t len = std::string::npos) const;
            /**
             * Appends a string to the end of the content.
             *
             * @param s
             * The character pointer to the string to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(const char *s);
            /**
             * Appends a string to the end of the content.
             *
             * @param s
             * The character pointer to the string to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(const char *s) const;
            /**
             * Appends a string to the end of the content.
             *
             * @param s
             * The character pointer to the string to append.
             *
             * @param n
             * The length of the string \a s.
             *
             * @return
             * A pointer to self.
             */
            String& append(const char *s, size_t n);
            /**
             * Appends a string to the end of the content.
             *
             * @param s
             * The character pointer to the string to append.
             *
             * @param n
             * The length of the string \a s.
             *
             * @return
             * A pointer to self.
             */
            String& append(const char *s, size_t n) const;
            /**
             * Appends a string to the end of the content.
             *
             * @param str
             * The class \b std::string to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(const std::string& str);
            /**
             * Appends a string to the end of the content.
             *
             * @param str
             * The class \b std::string to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(const std::string& str) const;
            /**
             * Appends a string to the end of the content.
             *
             * @param c
             * The character to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(char c);
            /**
             * Appends a string to the end of the content.
             *
             * @param c
             * The character to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(char c) const;
            /**
             * Appends a string to the end of the content.
             *
             * @param i
             * The integer to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(int i);
            /**
             * Appends a string to the end of the content.
             *
             * @param i
             * The integer to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(int i) const;
            /**
             * Appends a string to the end of the content.
             *
             * @param l
             * The long value to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(LONG l);
            /**
             * Appends a string to the end of the content.
             *
             * @param l
             * The long value to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(LONG l) const;
            /**
             * Appends a string to the end of the content.
             *
             * @param i
             * The unsigned integer to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(unsigned int i);
            /**
             * Appends a string to the end of the content.
             *
             * @param i
             * The unsigned integer to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(unsigned int i) const;
            /**
             * Appends a string to the end of the content.
             *
             * @param l
             * The unsigned long to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(ULONG l);
            /**
             * Appends a string to the end of the content.
             *
             * @param l
             * The unsigned long to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(ULONG l) const;
            /**
             * Appends a string to the end of the content.
             *
             * @param f
             * The float value to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(float f);
            /**
             * Appends a string to the end of the content.
             *
             * @param f
             * The float value to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(float f) const;
            /**
             * Appends a string to the end of the content.
             *
             * @param d
             * The double value to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(double d);
            /**
             * Appends a string to the end of the content.
             *
             * @param d
             * The double value to append.
             *
             * @return
             * A pointer to self.
             */
            String& append(double d) const;

            /**
             * Compares a string exactly with the internal buffer.
             *
             * @param str
             * The string to compare to.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int compare(const String& str);
            /**
             * Compares a string exactly with the internal buffer.
             *
             * @param str
             * The string to compare to.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int compare(const String& str) const;
            /**
             * Compares a string exactly with the internal buffer.
             * Only the length \a len is compared.
             *
             * @param str
             * The string to compare to.
             *
             * @param len
             * The length to compare.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int compare(const String& str, size_t len);
            /**
             * Compares a string exactly with the internal buffer.
             * Only the length \a len is compared.
             *
             * @param str
             * The string to compare to.
             *
             * @param len
             * The length to compare.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int compare(const String& str, size_t len) const;
            /**
             * Compares a string exactly with the internal buffer.
             *
             * @param s
             * The character pointer to the string to compare to.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int compare(const char *s);
            /**
             * Compares a string exactly with the internal buffer.
             *
             * @param s
             * The character pointer to the string to compare to.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int compare(const char *s) const;
            /**
             * Compares a string exactly with the internal buffer.
             *
             * @param s
             * The character pointer to the string to compare to.
             *
             * @param n
             * The length of the character string \a s.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int compare(const char *s, size_t n);
            /**
             * Compares a string exactly with the internal buffer.
             *
             * @param s
             * The character pointer to the string to compare to.
             *
             * @param n
             * The length of the character string \a s.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int compare(const char *s, size_t n) const;
            /**
             * Compares a string exactly with the internal buffer.
             *
             * @param str
             * The string to compare to.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int compare(const std::string& str);
            /**
             * Compares a string exactly with the internal buffer.
             *
             * @param str
             * The string to compare to.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int compare(const std::string& str) const;

            /**
             * Compares a string ignoring the case of the letters with the internal
             * buffer.
             *
             * @param str
             * The string to compare to.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int caseCompare(const String& str);
            /**
             * Compares a string ignoring the case of the letters with the internal
             * buffer.
             *
             * @param str
             * The string to compare to.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int caseCompare(const String& str) const;
            /**
             * Compares a string ignoring the case of the letters with the internal
             * buffer.
             *
             * @param str
             * The string to compare to.
             *
             * @param len
             * The length to compare to.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int caseCompare(const String& str, size_t len);
            /**
             * Compares a string ignoring the case of the letters with the internal
             * buffer.
             *
             * @param str
             * The string to compare to.
             *
             * @param len
             * The length to compare to.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int caseCompare(const String& str, size_t len) const;
            /**
             * Compares a string ignoring the case of the letters with the internal
             * buffer.
             *
             * @param s
             * The character pointer to the string to compare to.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int caseCompare(const char *s);
            /**
             * Compares a string ignoring the case of the letters with the internal
             * buffer.
             *
             * @param s
             * The character pointer to the string to compare to.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int caseCompare(const char *s) const;
            /**
             * Compares a string ignoring the case of the letters with the internal
             * buffer.
             *
             * @param s
             * The character pointer to the string to compare to.
             *
             * @param n
             * The length of the string \a s.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int caseCompare(const char *s, size_t n);
            /**
             * Compares a string ignoring the case of the letters with the internal
             * buffer.
             *
             * @param s
             * The character pointer to the string to compare to.
             *
             * @param n
             * The length of the string \a s.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int caseCompare(const char *s, size_t n) const;
            /**
             * Compares a string ignoring the case of the letters with the internal
             * buffer.
             *
             * @param str
             * The string to compare to.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int caseCompare(const std::string& str);
            /**
             * Compares a string ignoring the case of the letters with the internal
             * buffer.
             *
             * @param str
             * The string to compare to.
             *
             * @return
             * If the strings are equal, 0 ist returned. Otherwise a value not
             * equal to 0 is returned.
             */
            int caseCompare(const std::string& str) const;

            /**
             * Finds the first occurence of \a str in the internal buffer, where
             * it starts from index \a pos.
             *
             * @param str
             * The sequence to find.
             *
             * @param pos
             * Optional: The index position of where to start from. This index
             * must not be grater than \b length().
             *
             * @return
             * The index where the sequence was found, or \b std::string::npos
             * if the sequence was not found.
             */
            size_t findOf(const String& str, size_t pos=0);
            /**
             * Finds the first occurence of \a s in the internal buffer, where
             * it starts from index \a pos.
             *
             * @param s
             * The sequence to find.
             *
             * @param pos
             * Optional: The index position of where to start from. This index
             * must not be grater than \b length().
             *
             * @return
             * The index where the sequence was found, or \b std::string::npos
             * if the sequence was not found.
             */
            size_t findOf(const char *s, size_t pos=0);
            /**
             * Finds the first occurence of \a s in the internal buffer, where
             * it starts from index \a pos.
             *
             * @param s
             * The character to find.
             *
             * @param pos
             * Optional: The index position of where to start from. This index
             * must not be grater than \b length().
             *
             * @return
             * The index where the character was found, or \b std::string::npos
             * if the sequence was not found.
             */
            size_t findOf(char s, size_t pos=0);
            /**
             * Finds the first occurence of \a str in the internal buffer, where
             * it starts from index \a pos.
             *
             * @param str
             * The sequence to find.
             *
             * @param pos
             * Optional: The index position of where to start from. This index
             * must not be grater than \b length().
             *
             * @return
             * The index where the sequence was found, or \b std::string::npos
             * if the sequence was not found.
             */
            size_t findOf(const std::string& str, size_t pos=0);

            /**
             * Finds the first occurence of \a str in the internal buffer, where
             * it starts from index \a pos.
             *
             * @param str
             * The sequence to find.
             *
             * @param pos
             * Optional: The index position of where to start from. This index
             * must not be grater than \b length().
             *
             * @return
             * The index where the sequence was found, or \b std::string::npos
             * if the sequence was not found.
             */
            size_t findOf(const String& str, size_t pos=0) const;
            /**
             * Finds the first occurence of \a s in the internal buffer, where
             * it starts from index \a pos.
             *
             * @param s
             * The sequence to find.
             *
             * @param pos
             * Optional: The index position of where to start from. This index
             * must not be grater than \b length().
             *
             * @return
             * The index where the sequence was found, or \b std::string::npos
             * if the sequence was not found.
             */
            size_t findOf(const char *s, size_t pos=0) const;
            /**
             * Finds the first occurence of \a s in the internal buffer, where
             * it starts from index \a pos.
             *
             * @param s
             * The character to find.
             *
             * @param pos
             * Optional: The index position of where to start from. This index
             * must not be grater than \b length().
             *
             * @return
             * The index where the character was found, or \b std::string::npos
             * if the sequence was not found.
             */
            size_t findOf(char s, size_t pos=0) const;
            /**
             * Finds the first occurence of \a str in the internal buffer, where
             * it starts from index \a pos.
             *
             * @param str
             * The sequence to find.
             *
             * @param pos
             * Optional: The index position of where to start from. This index
             * must not be grater than \b length().
             *
             * @return
             * The index where the sequence was found, or \b std::string::npos
             * if the sequence was not found.
             */
            size_t findOf(const std::string& str, size_t pos=0) const;

            /**
             * Finds the first occurence of \a str in the internal buffer.
             *
             * @param str
             * The sequence to find.
             *
             * @return
             * The index where the sequence was found, or \b std::string::npos
             * if the sequence was not found.
             */
            size_t findFirstOf(const String& str);
            /**
             * Finds the first occurence of \a s in the internal buffer.
             *
             * @param s
             * The sequence to find.
             *
             * @return
             * The index where the sequence was found, or \b std::string::npos
             * if the sequence was not found.
             */
            size_t findFirstOf(const char *s);
            /**
             * Finds the first occurence of \a s in the internal buffer.
             *
             * @param s
             * The character to find.
             *
             * @return
             * The index where the character was found, or \b std::string::npos
             * if the sequence was not found.
             */
            size_t findFirstOf(char s);
            /**
             * Finds the first occurence of \a str in the internal buffer.
             *
             * @param str
             * The sequence to find.
             *
             * @return
             * The index where the sequence was found, or \b std::string::npos
             * if the sequence was not found.
             */
            size_t findFirstOf(const std::string& str);

            /**
             * Returns the index of the first occurrence of a string in self that
             * is not \a str.
             *
             * @param str
             * The string not wanted.
             *
             * @return
             * The index or \b std::string::npos
             * if there is no match.
             */
            size_t findFirstNotOf(const String& str);
            /**
             * Returns the index of the first occurrence of a string in self that
             * is not \a s.
             *
             * @param s
             * The string not wanted.
             *
             * @return
             * The index or \b std::string::npos
             * if there is no match.
             */
            size_t findFirstNotOf(const char *s);
             /**
             * Returns the index of the first occurrence of a charcter in self that
             * is not \a s.
             *
             * @param s
             * The character not wanted.
             *
             * @return
             * The index or \b std::string::npos
             * if there is no match.
             */
            size_t findFirstNotOf(char s);
            /**
             * Returns the index of the first occurrence of a string in self that
             * is not \a str.
             *
             * @param str
             * The string not wanted.
             *
             * @return
             * The index or \b std::string::npos
             * if there is no match.
             */
            size_t findFirstNotOf(const std::string& str);

            /**
             * Returns the index of the last occurrence of a string in self that
             * matches \a str.
             *
             * @param str
             * The string to search for.
             *
             * @return
             * The index of the string or \b std::string::npos
             * if there is no match.
             */
            size_t findLastOf(const String& str);
            /**
             * Returns the index of the last occurrence of a string in self that
             * matches \a s.
             *
             * @param s
             * The string to search for.
             *
             * @return
             * The index of the string or \b std::string::npos
             * if there is no match.
             */
            size_t findLastOf(const char *s);
            /**
             * Returns the index of the last occurrence of a charcter in self that
             * matches \a c.
             *
             * @param s
             * The charcter to search for.
             *
             * @return
             * The index of the character or \b std::string::npos
             * if there is no match.
             */
            size_t findLastOf(char s);
            /**
             * Returns the index of the last occurrence of a string in self that
             * matches \a str.
             *
             * @param str
             * The string to search for.
             *
             * @return
             * The index of the string or \b std::string::npos
             * if there is no match.
             */
            size_t findLastOf(const std::string& str);

            /**
             * Starting at \a start, returns a substring of the length \a len,
             * or the rest of the string.
             *
             * @param start
             * The index where the substring should start. This must be less
             * then \b length(). This function does not alter the internal
             * buffer!
             *
             * @param len
             * Optional: If the length is grater than 0, a substring with the
             * length \a len is returned, or, if the rest of the string is less
             * than \a len, the rest of the string will be selected.
             * If this parameter is omitted or equal to 0, the rest of the
             * string, starting from \a start ist returned.
             *
             * @return
             * A pointer to a \a String class containing the new part of the
             * original string.
             */
            String substring(size_t start, size_t len=0);
            /**
             * Starting at \a start, returns a substring of the length \a len,
             * or the rest of the string \a str.
             *
             * @param str
             * The string from where the substring should be returned.
             *
             * @param start
             * The index where the substring should start. This must be less
             * then \b length(). This function alters the internal buffer!
             *
             * @param len
             * Optional: If the length is grater than 0, a substring with the
             * length \a len is returned, or, if the rest of the string is less
             * than \a len, the rest of the string will be selected.
             *
             * @return
             * A pointer to self containing the new part of the original string.
             * The original string is lost.
             */
            String& substring(String& str, size_t start, size_t len=0);
            /**
             * Starting from the first occurence of \a pat in self a string up
             * to \a len bytes is returned or the rest of the string if \a len
             * is omitted.
             *
             * @param pat
             * The string where to start the substring.
             *
             * @param len
             * Optional: If the length is grater than 0, a substring with the
             * length \a len is returned, or, if the rest of the string is less
             * than \a len, the rest of the string will be selected.
             *
             * @return
             * A pointer to the new substring.
             */
            String substring(const String& pat, size_t len=0);
            /**
             * Starting from the first occurence of \a pat in self a string up
             * to \a len bytes is returned or the rest of the string if \a len
             * is omitted.
             *
             * @param pat
             * The string where to start the substring.
             *
             * @param len
             * Optional: If the length is grater than 0, a substring with the
             * length \a len is returned, or, if the rest of the string is less
             * than \a len, the rest of the string will be selected.
             *
             * @return
             * A pointer to the new substring.
             */
            String substring(const char *pat, size_t len=0);
            /**
             * Starting from the first occurence of \a pat in self a string up
             * to \a len bytes is returned or the rest of the string if \a len
             * is omitted.
             *
             * @param pat
             * The string where to start the substring.
             *
             * @param len
             * Optional: If the length is grater than 0, a substring with the
             * length \a len is returned, or, if the rest of the string is less
             * than \a len, the rest of the string will be selected.
             *
             * @return
             * A pointer to the new substring.
             */
            String substring(const std::string& pat, size_t len=0);

            /**
             * Starting at \a start, returns a substring of the length \a len,
             * or the rest of the string.
             *
             * @param start
             * The index where the substring should start. This must be less
             * then \b length(). This function does not alter the internal
             * buffer!
             *
             * @param len
             * Optional: If the length is grater than 0, a substring with the
             * length \a len is returned, or, if the rest of the string is less
             * than \a len, the rest of the string will be selected.
             * If this parameter is omitted or equal to 0, the rest of the
             * string, starting from \a start ist returned.
             *
             * @return
             * A pointer to a \a String class containing the new part of the
             * original string.
             */
            String substring(size_t start, size_t len=0) const;
            /**
             * Starting at \a start, returns a substring of the length \a len,
             * or the rest of the string \a str.
             *
             * @param str
             * The string from where the substring should be returned.
             *
             * @param start
             * The index where the substring should start. This must be less
             * then \b length(). This function alters the internal buffer!
             *
             * @param len
             * Optional: If the length is grater than 0, a substring with the
             * length \a len is returned, or, if the rest of the string is less
             * than \a len, the rest of the string will be selected.
             *
             * @return
             * A pointer to self containing the new part of the original string.
             * The original string is lost.
             */
            String& substring(String& str, size_t start, size_t len=0) const;
            /**
             * Starting from the first occurence of \a pat in self a string up
             * to \a len bytes is returned or the rest of the string if \a len
             * is omitted.
             *
             * @param pat
             * The string where to start the substring.
             *
             * @param len
             * Optional: If the length is grater than 0, a substring with the
             * length \a len is returned, or, if the rest of the string is less
             * than \a len, the rest of the string will be selected.
             *
             * @return
             * A pointer to the new substring.
             */
            String substring(const String& pat, size_t len=0) const;
            /**
             * Starting from the first occurence of \a pat in self a string up
             * to \a len bytes is returned or the rest of the string if \a len
             * is omitted.
             *
             * @param pat
             * The string where to start the substring.
             *
             * @param len
             * Optional: If the length is grater than 0, a substring with the
             * length \a len is returned, or, if the rest of the string is less
             * than \a len, the rest of the string will be selected.
             *
             * @return
             * A pointer to the new substring.
             */
            String substring(const char *pat, size_t len=0) const;
            /**
             * Starting from the first occurence of \a pat in self a string up
             * to \a len bytes is returned or the rest of the string if \a len
             * is omitted.
             *
             * @param pat
             * The string where to start the substring.
             *
             * @param len
             * Optional: If the length is grater than 0, a substring with the
             * length \a len is returned, or, if the rest of the string is less
             * than \a len, the rest of the string will be selected.
             *
             * @return
             * A pointer to the new substring.
             */
            String substring(const std::string& pat, size_t len=0) const;

            /**
             * Cats all blanks and tabulators from the start of the string in
             * self.
             *
             * @return
             * A pointer to self.
             */
            String& ltrim();
            /**
             * Cats all blanks and tabulators from the end of the string in
             * self.
             *
             * @return
             * A pointer to self.
             */
            String& rtrim();
            /**
             * Cats all blanks and tabulators from the start and the end of the
             * string in self.
             *
             * @return
             * A pointer to self.
             */
            String& trim() { ltrim(); return rtrim(); }
            /**
             * Cats all blanks and tabulators from the start of the string in
             * self.
             *
             * @return
             * A pointer to self.
             */
            String& ltrim() const;
            /**
             * Cats all blanks and tabulators from the end of the string in
             * self.
             *
             * @return
             * A pointer to self.
             */
            String& rtrim() const;
            /**
             * Cats all blanks and tabulators from the start and the end of the
             * string in self.
             *
             * @return
             * A pointer to self.
             */
            String& trim() const { ltrim(); return rtrim(); }

            /**
             * Searches for the occurences of \a seperator in self and returns
             * a list of string parts between the found \a seperator.
             *
             * @param seperator
             * The character which is used to seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(char seperator);
            /**
             * Searches for the occurences of \a seperator in self and returns
             * a list of string parts between the found \a seperator.
             *
             * @param seperator
             * The character which is used to seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(char seperator) const;
            /**
             * Searches for the occurences of \a seperator in self and returns
             * a list of string parts between the found \a seperator.
             *
             * @param seperator
             * The character which is used to seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(int seperator);
            /**
             * Searches for the occurences of \a seperator in self and returns
             * a list of string parts between the found \a seperator.
             *
             * @param seperator
             * The character which is used to seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(int seperator) const;
            /**
             * Searches for the occurences of characters defined in \a seps, in
             * self and returns a list of string parts between the found
             * characters in \a seps.
             *
             * @param seps
             * A string containing one or more characters which are used to
             * seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(const String& seps);
            /**
             * Searches for the occurences of characters defined in \a seps, in
             * self and returns a list of string parts between the found
             * characters in \a seps.
             *
             * @param seps
             * A string containing one or more characters which are used to
             * seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(const String& seps) const;
            /**
             * Searches for the occurences of characters defined in \a seps, in
             * self and returns a list of string parts between the found
             * characters in \a seps.
             *
             * @param seps
             * A string containing one or more characters which are used to
             * seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(const char *seps);
            /**
             * Searches for the occurences of characters defined in \a seps, in
             * self and returns a list of string parts between the found
             * characters in \a seps.
             *
             * @param seps
             * A string containing one or more characters which are used to
             * seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(const char *seps) const;
            /**
             * Searches for the occurences of characters defined in \a seps, in
             * self and returns a list of string parts between the found
             * characters in \a seps.
             *
             * @param seps
             * A string containing one or more characters which are used to
             * seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(const std::string& seps);
            /**
             * Searches for the occurences of characters defined in \a seps, in
             * self and returns a list of string parts between the found
             * characters in \a seps.
             *
             * @param seps
             * A string containing one or more characters which are used to
             * seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(const std::string& seps) const;

            /**
             * Searches for the occurences of \a seperator in \a s and returns
             * a list of string parts between the found \a seperator.
             *
             * @param s
             * The string which is splitted into parts delimitted by the
             * character in \a seperator.
             *
             * @param seperator
             * The character which is used to seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(const String& s, char seperator);
            /**
             * Searches for the occurences of \a seperator in \a s and returns
             * a list of string parts between the found \a seperator.
             *
             * @param s
             * The string which is splitted into parts delimitted by the
             * character in \a seperator.
             *
             * @param seperator
             * The character which is used to seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(const String& s, char seperator) const;
            /**
             * Searches for the occurences of \a seperator in \a s and returns
             * a list of string parts between the found \a seperator.
             *
             * @param s
             * The string which is splitted into parts delimitted by the
             * character in \a seperator.
             *
             * @param seperator
             * The character which is used to seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(const char *s, char seperator);
            /**
             * Searches for the occurences of \a seperator in \a s and returns
             * a list of string parts between the found \a seperator.
             *
             * @param s
             * The string which is splitted into parts delimitted by the
             * character in \a seperator.
             *
             * @param seperator
             * The character which is used to seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(const char *s, char seperator) const;
            /**
             * Searches for the occurences of \a seperator in \a s and returns
             * a list of string parts between the found \a seperator.
             *
             * @param s
             * The string which is splitted into parts delimitted by the
             * character in \a seperator.
             *
             * @param seperator
             * The character which is used to seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(const std::string& s, char seperator);
            /**
             * Searches for the occurences of \a seperator in \a s and returns
             * a list of string parts between the found \a seperator.
             *
             * @param s
             * The string which is splitted into parts delimitted by the
             * character in \a seperator.
             *
             * @param seperator
             * The character which is used to seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(const std::string& s, char seperator) const;

            /**
             * Searches for the occurences of characters defined in \a seps, in
             * \a s and returns a list of string parts between the found
             * characters in \a seps.
             *
             * @param s
             * The string which is splitted into parts delimitted by a
             * character in \a seps.
             *
             * @param seps
             * A string containing one or more characters which are used to
             * seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(const String& s, const String& seps);
            /**
             * Searches for the occurences of characters defined in \a seps, in
             * \a s and returns a list of string parts between the found
             * characters in \a seps.
             *
             * @param s
             * The string which is splitted into parts delimitted by a
             * character in \a seps.
             *
             * @param seps
             * A string containing one or more characters which are used to
             * seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(const String& s, const String& seps) const;
            /**
             * Searches for the occurences of characters defined in \a seps, in
             * \a s and returns a list of string parts between the found
             * characters in \a seps.
             *
             * @param s
             * The string which is splitted into parts delimitted by a
             * character in \a seps.
             *
             * @param seps
             * A string containing one or more characters which are used to
             * seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(const char *s, const char *seps);
            /**
             * Searches for the occurences of characters defined in \a seps, in
             * \a s and returns a list of string parts between the found
             * characters in \a seps.
             *
             * @param s
             * The string which is splitted into parts delimitted by a
             * character in \a seps.
             *
             * @param seps
             * A string containing one or more characters which are used to
             * seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(const char *s, const char *seps) const;
            /**
             * Searches for the occurences of characters defined in \a seps, in
             * \a s and returns a list of string parts between the found
             * characters in \a seps.
             *
             * @param s
             * The string which is splitted into parts delimitted by a
             * character in \a seps.
             *
             * @param seps
             * A string containing one or more characters which are used to
             * seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(const std::string& s, const std::string& seps);
            /**
             * Searches for the occurences of characters defined in \a seps, in
             * \a s and returns a list of string parts between the found
             * characters in \a seps.
             *
             * @param s
             * The string which is splitted into parts delimitted by a
             * character in \a seps.
             *
             * @param seps
             * A string containing one or more characters which are used to
             * seperate the string parts.
             *
             * @return
             * A vector list with objects of type String.
             */
            std::vector<String> split(const std::string& s, const std::string& seps) const;

            /**
             * Searches for the occurence of \a str in self and replaces it with
             * \a what.
             * By default all occurences of \a str in self are replaced by \a what.
             * If the optional parmeter \a rep is set, all, the first or the
             * last occurence is replaced.
             *
             * @param str
             * The pattern to search for in self.
             *
             * @param what
             * The pattern to replace \a str in self.
             *
             * @param rep
             * Optional: This can be ALL, FIRST or LAST.
             * ALL means to replace all occurenaces of \a str in self.
             * FIRST means to replace only the first occurence of \a str in self.
             * LAST means to replace only the last occurence of \a str in self.
             *
             * @return
             * A pointer to self.
             */
            String& replace(const String& str, const String& what, REPLACE rep=ALL);
            /**
             * Searches for the occurence of \a str in self and replaces it with
             * \a what.
             * By default all occurences of \a str in self are replaced by \a what.
             * If the optional parmeter \a rep is set, all, the first or the
             * last occurence is replaced.
             *
             * @param str
             * The pattern to search for in self.
             *
             * @param what
             * The pattern to replace \a str in self.
             *
             * @param rep
             * Optional: This can be ALL, FIRST or LAST.
             * ALL means to replace all occurenaces of \a str in self.
             * FIRST means to replace only the first occurence of \a str in self.
             * LAST means to replace only the last occurence of \a str in self.
             *
             * @return
             * A pointer to self.
             */
            String& replace(const char *str, const char *what, REPLACE rep=ALL);
            /**
             * Searches for the occurence of \a str in self and replaces it with
             * \a what.
             * By default all occurences of \a str in self are replaced by \a what.
             * If the optional parmeter \a rep is set, all, the first or the
             * last occurence is replaced.
             *
             * @param str
             * The pattern to search for in self.
             *
             * @param what
             * The pattern to replace \a str in self.
             *
             * @param rep
             * Optional: This can be ALL, FIRST or LAST.
             * ALL means to replace all occurenaces of \a str in self.
             * FIRST means to replace only the first occurence of \a str in self.
             * LAST means to replace only the last occurence of \a str in self.
             *
             * @return
             * A pointer to self.
             */
            String& replace(const std::string& str, const std::string& what, REPLACE rep=ALL);

            /**
             * Insert a string \a str at the index position \a pos in self. The
             * index position \a pos must not be grater than \b length().
             *
             * @param pos
             * The index position wher to insert the string \a str.
             *
             * @param str
             * The string to insert at index position \a pos.
             *
             * @return
             * A pointer to self.
             */
            String& insert(size_t pos, const String& str);
            /**
             * Insert a string \a str at the index position \a pos in self. The
             * index position \a pos must not be grater than \b length().
             *
             * @param pos
             * The index position wher to insert the string \a str.
             *
             * @param str
             * The string to insert at index position \a pos.
             *
             * @return
             * A pointer to self.
             */
            String& insert(size_t pos, const std::string& str);
            /**
             * Insert a string \a s at the index position \a pos in self. The
             * index position \a pos must not be grater than \b length().
             *
             * @param pos
             * The index position wher to insert the string \a s.
             *
             * @param s
             * The string to insert at index position \a pos.
             *
             * @return
             * A pointer to self.
             */
            String& insert(size_t pos, const char *s);

            /**
             * This should be used as an argument to the string in the internal
             * buffer. If the string in the internal buffer contains one or more
             * placeholder of the form "%1", where the number marks the argument
             * number, then this marker will be replaced by the argument.
             *
             * @code
             * String s("There are %1 of %2 in the valley").arg(3).arg("houses");
             * @endcode
             *
             * This results in an output of:
             * @code
             * There are 3 of houses in the valley
             * @endcode
             *
             * @param str
             * The string to insert instead of the place holder.
             *
             * @return
             * A pointer to self.
             */
            String& arg(const String& str);
            /**
             * This should be used as an argument to the string in the internal
             * buffer. If the string in the internal buffer contains one or more
             * placeholder of the form "%1", where the number marks the argument
             * number, then this marker will be replaced by the argument.
             *
             * @code
             * String s("There are %1 of %2 in the valley").arg(3).arg("houses");
             * @endcode
             *
             * This results in an output of:
             * @code
             * There are 3 of houses in the valley
             * @endcode
			 *
             * @param str
             * The string to insert instead of the place holder.
             *
             * @return
             * A pointer to self.
             */
            String& arg(const std::string& str);
            /**
             * This should be used as an argument to the string in the internal
             * buffer. If the string in the internal buffer contains one or more
             * placeholder of the form "%1", where the number marks the argument
             * number, then this marker will be replaced by the argument.
             *
             * @code
             * String s("There are %1 of %2 in the valley").arg(3).arg("houses");
             * @endcode
             *
             * This results in an output of:
             * @code
             * There are 3 of houses in the valley
             * @endcode
			 *
             * @param str
             * The string to insert instead of the place holder.
             *
             * @return
             * A pointer to self.
             */
            String& arg(const char *str);
            /**
             * This should be used as an argument to the string in the internal
             * buffer. If the string in the internal buffer contains one or more
             * placeholder of the form "%1", where the number marks the argument
             * number, then this marker will be replaced by the argument.
             *
             * @code
             * String s("There are %1 of %2 in the valley").arg(3).arg("houses");
             * @endcode
             *
             * This results in an output of:
             * @code
             * There are 3 of houses in the valley
             * @endcode
			 *
             * @param c
             * The character to insert instead of the place holder.
             *
             * @return
             * A pointer to self.
             */
            String& arg(const char c);
            /**
             * This should be used as an argument to the string in the internal
             * buffer. If the string in the internal buffer contains one or more
             * placeholder of the form "%1", where the number marks the argument
             * number, then this marker will be replaced by the argument.
             *
             * @code
             * String s("There are %1 of %2 in the valley").arg(3).arg("houses");
             * @endcode
             *
             * This results in an output of:
             * @code
             * There are 3 of houses in the valley
             * @endcode
			 *
             * @param i
             * The integer to insert instead of the place holder.
             *
             * @return
             * A pointer to self.
             */
            String& arg(const int i);
            /**
             * This should be used as an argument to the string in the internal
             * buffer. If the string in the internal buffer contains one or more
             * placeholder of the form "%1", where the number marks the argument
             * number, then this marker will be replaced by the argument.
             *
             * @code
             * String s("There are %1 of %2 in the valley").arg(3).arg("houses");
             * @endcode
             *
             * This results in an output of:
             * @code
             * There are 3 of houses in the valley
             * @endcode
			 *
             * @param i
             * The unsigned integer to insert instead of the place holder.
             *
             * @return
             * A pointer to self.
             */
            String& arg(const unsigned int i);
            /**
             * This should be used as an argument to the string in the internal
             * buffer. If the string in the internal buffer contains one or more
             * placeholder of the form "%1", where the number marks the argument
             * number, then this marker will be replaced by the argument.
             *
             * @code
             * String s("There are %1 of %2 in the valley").arg(3).arg("houses");
             * @endcode
             *
             * This results in an output of:
             * @code
             * There are 3 of houses in the valley
             * @endcode
			 *
             * @param l
             * The long value to insert instead of the place holder.
             *
             * @return
             * A pointer to self.
             */
            String& arg(const LONG l);
            /**
             * This should be used as an argument to the string in the internal
             * buffer. If the string in the internal buffer contains one or more
             * placeholder of the form "%1", where the number marks the argument
             * number, then this marker will be replaced by the argument.
             *
             * @code
             * String s("There are %1 of %2 in the valley").arg(3).arg("houses");
             * @endcode
             *
             * This results in an output of:
             * @code
             * There are 3 of houses in the valley
             * @endcode
			 *
             * @param l
             * The unsigned long value to insert instead of the place holder.
             *
             * @return
             * A pointer to self.
             */
            String& arg(const ULONG l);
            /**
             * This should be used as an argument to the string in the internal
             * buffer. If the string in the internal buffer contains one or more
             * placeholder of the form "%1", where the number marks the argument
             * number, then this marker will be replaced by the argument.
             *
             * @code
             * String s("There are %1 of %2 in the valley").arg(3).arg("houses");
             * @endcode
             *
             * This results in an output of:
             * @code
             * There are 3 of houses in the valley
             * @endcode
			 *
             * @param f
             * The float value to insert instead of the place holder.
             *
             * @return
             * A pointer to self.
             */
            String& arg(const float f);
            /**
             * This should be used as an argument to the string in the internal
             * buffer. If the string in the internal buffer contains one or more
             * placeholder of the form "%1", where the number marks the argument
             * number, then this marker will be replaced by the argument.
             *
             * @code
             * String s("There are %1 of %2 in the valley").arg(3).arg("houses");
             * @endcode
             *
             * This results in an output of:
             * @code
             * There are 3 of houses in the valley
             * @endcode
			 *
             * @param f
             * The double value to insert instead of the place holder.
             *
             * @return
             * A pointer to self.
             */
            String& arg(const double f);

            /**
             * Checks whether the string \a str is contained in self or not.
             *
             * @param str
             * The string to search for in self.
             *
             * @return
             * TRUE if the string \a str is found in self. FALS if it is not
             * found.
             */
            bool contains(const String& str) { return _find(_string, str._string, str._len) == 0; }
            /**
             * Checks whether the string \a str is contained in self or not.
             *
             * @param str
             * The string to search for in self.
             *
             * @return
             * TRUE if the string \a str is found in self. FALS if it is not
             * found.
             */
            bool contains(const std::string& str) { return _find(_string, str.c_str(), str.size()) == 0; }
            /**
             * Checks whether the string \a s is contained in self or not.
             *
             * @param s
             * The string to search for in self.
             *
             * @return
             * TRUE if the string \a s is found in self. FALS if it is not
             * found.
             */
            bool contains(const char *s) { return _find(_string, s, strlen(s)) == 0; }

            /**
             * Exchanges the content of the container by the content of str,
             * which is another String object. Lengths may differ.
             *
             * After the call to this member function, the value of this object
             * is the value str had before the call, and the value of str is the
             * value this object had before the call.
             *
             * @param str
             * Another String object, whose value is swapped with that of this
             * String.
             */
            void swap(String& str);

            char operator[] (size_t pos)
            {
                if (pos < _len)
                    return *(_string+pos);

                return 0;
            }

            char operator[] (size_t pos) const
            {
                if (pos < _len)
                    return *(_string+pos);

                return 0;
            }

            String& operator+= (const String& str)      { return append(str); }
            String& operator+= (const std::string& str) { return append(str); }
            String& operator+= (const char* s)          { return append(s); }
            String& operator+= (char c)                 { return append(c); }
            String& operator+= (int i)                  { return append(i); }
            String& operator+= (LONG l)                 { return append(l); }
            String& operator+= (unsigned int i)         { return append(i); }
            String& operator+= (ULONG l)                { return append(l); }
            String& operator+= (float f)                { return append(f); }
            String& operator+= (double d)               { return append(d); }

            String operator+ (const String& str)       { return appendVirtual(str); }
            String operator+ (const std::string& str)  { return appendVirtual(str); }
            String operator+ (const char* s)           { return appendVirtual(s); }
            String operator+ (char c)                  { return appendVirtual(c); }
            String operator+ (int i)                   { return appendVirtual(i); }
            String operator+ (LONG l)                  { return appendVirtual(l); }
            String operator+ (unsigned int i)          { return appendVirtual(i); }
            String operator+ (ULONG l)                 { return appendVirtual(l); }
            String operator+ (float f)                 { return appendVirtual(f); }
            String operator+ (double d)                { return appendVirtual(d); }

            String operator+ (const String& str) const      { return appendVirtual(str); }
            String operator+ (const std::string& str) const { return appendVirtual(str); }
            String operator+ (const char* s) const          { return appendVirtual(s); }
            String operator+ (char c) const                 { return appendVirtual(c); }
            String operator+ (int i) const                  { return appendVirtual(i); }
            String operator+ (LONG l) const                 { return appendVirtual(l); }
            String operator+ (unsigned int i) const         { return appendVirtual(i); }
            String operator+ (ULONG l) const                { return appendVirtual(l); }
            String operator+ (float f) const                { return appendVirtual(f); }
            String operator+ (double d) const               { return appendVirtual(d); }

            String& operator= (const String& str)       { return assign(str); }
            String& operator= (const std::string& str)  { return assign(str); }
            String& operator= (const char* s)           { return assign(s); }
            String& operator= (char c)                  { return assign(c); }
            String& operator= (int i)                   { return assign(i); }
            String& operator= (LONG l)                  { return assign(l); }
            String& operator= (unsigned int i)          { return assign(i); }
            String& operator= (ULONG l)                 { return assign(l); }
            String& operator= (float f)                 { return assign(f); }
            String& operator= (double d)                { return assign(d); }

            bool operator== (const String& str)   { return (_comp(_string, (char *)str._string, (_len > str._len)?_len:str._len) == 0); }
            bool operator== (const char* s)       { return (_comp(_string, (char *)s, _len) == 0); }
            bool operator== (const std::string& str) { return (_comp(_string, (char *)str.c_str(), (_len > str.size())?_len:str.size()) == 0); }
            bool operator== (const String& str) const  { return (_comp(_string, (char *)str._string, (_len > str._len)?_len:str._len) == 0); }
            bool operator== (const char* s) const      { return (_comp(_string, (char *)s, _len) == 0); }
            bool operator== (const std::string& str) const { return (_comp(_string, (char *)str.c_str(), (_len > str.size())?_len:str.size()) == 0); }

            friend std::ostream& operator<< (std::ostream& os, const String& str) { return os << str.toString(); }
            friend std::ostream& operator<< (std::ostream& os, const std::string& str) { return os << str; }
            friend std::ostream& operator<< (std::ostream& os, const char *s) { return os << s; }

            friend std::istream& operator>> (std::istream& in, String& str)
            {
                std::string s;
                in >> s;
                str.assign(s);
                return in;
            }

        protected:
            /**
             * \internal
             * Verifies that the index \a i is within the bounds of the substring.
             *
             * \exception ERR_OUT_OF_BOUNDS
             * if the index is out of range.
             */
            void assertElement(size_t i) const;

            /**
             * \internal
             * Verifies that the memory to allocate is within the maximum
             * defined.
             *
             * \exception ERR_OUT_OF_MEMORY
             * Can't allocate more meory
             */
            void assertMemory(size_t i) const;

        private:
            void copyString(const char *s, size_t len);
            void appendString(const char *s, size_t len);
            void appendString(const char *s, size_t len) const;
            String appendVirtual(const String& s);
            String appendVirtual(const std::string& s);
            String appendVirtual(const char *s);
            String appendVirtual(const char c);
            String appendVirtual(const int i);
            String appendVirtual(const LONG l);
            String appendVirtual(const unsigned int i);
            String appendVirtual(const ULONG l);
            String appendVirtual(const float f);
            String appendVirtual(const double d);
            String appendVirtual(const String& s) const;
            String appendVirtual(const std::string& s) const;
            String appendVirtual(const char *s) const;
            String appendVirtual(const char c) const;
            String appendVirtual(const int i) const;
            String appendVirtual(const LONG l) const;
            String appendVirtual(const unsigned int i) const;
            String appendVirtual(const ULONG l) const;
            String appendVirtual(const float f) const;
            String appendVirtual(const double d) const;
            char *getMemory(size_t l);
            char *getMemory(size_t l) const;
            char *getMemory(size_t l, size_t& a, char *p);
            char *getMemory(size_t l, size_t& a, char *p) const;
            int _comp(char *s1, char *s2, size_t l);
            int _comp(char *s1, char *s2, size_t l) const;
            int _compN(char *s1, char *s2, size_t l);
            int _compN(char *s1, char *s2, size_t l) const;
            size_t _find(const char *s, const char *what, size_t len);
            size_t _find(const char *s, const char *what, size_t len) const;
            size_t _findNot(const char *s, const char *what, size_t len);
            size_t _find_last(const char *s, const char *what, size_t len);
            std::vector<String> _split(const String& str, const char *seps);
            std::vector<String> _split(const String& str, const char *seps) const;
            char *_replace(char *pattern, size_t patlen, char *replacement, size_t replen, REPLACE rep);
            size_t _max(size_t a1, size_t a2);
            size_t _min(size_t a1, size_t a2);
            size_t _max(size_t a1, size_t a2) const;
            size_t _min(size_t a1, size_t a2) const;
            size_t _isPart(const char *str, const char *p, size_t position);
            size_t _isPart(const char *str, const char *p, size_t position) const;
            bool _insert(size_t pos, const char *s, size_t len);
            std::istream& _rdToDelim(std::istream& stream, char delim);

            char *_string;              // The pointer to the container
            size_t _len;                // The length inside the container
            size_t _allocated;          // The length of the container
            int _replacePos;            // The position number where a string of type "%1" may be replaced
    };
}       // namespace strings

#endif

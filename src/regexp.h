/*
 *   Copyright (C) 2018 by Andreas Theofilu (TheoSys) <andreas@theosys.at>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

#ifndef __REGEXP_H__
#define __REGEXP_H__

#include <cstring>
#include <iostream>
#include <exception>
#include <sys/types.h>
#include <regex.h>
#include "strings.h"

#ifndef REG_NOERROR
#define REG_NOERROR     0
#endif
#ifndef REG_EEND
#define REG_EEND        14
#endif
#ifndef REG_ESIZE
#define REG_ESIZE       15
#endif
#ifndef REG_ERPAREN
#define REG_ERPAREN     16
#endif
#ifndef REG_STARTEND
#define REG_STARTEND    (1 << 2)
#endif

namespace strings
{
    /**
     * \addtogroup REGEXP
     * @{
     * \def MAX_REGERG
     * The maximum number of results out of executing a regular expression.
     */
    #define MAX_REGERG  100
    /** @} */

    class String;

    /**
     * \enum RegError
     * This are the error codes who may be happen. In case an error occures, an
     * exception of type \a RegException is thrown.
     * 
     * \brief Possible error codes throwing an exception.
     */
    enum RegError
    {
        ERR_NOERROR = REG_NOERROR,      //!< No error, everything ok
        ERR_NOMATCH = REG_NOMATCH,      //!< No error, but regular expression did not match
        ERR_BADBR = REG_BADBR,          //!< Invalid use of back reference operator.
        ERR_BADPAT = REG_BADPAT,        //!< Invalid use of pattern operators such as group or list.
        ERR_BADRPT = REG_BADRPT,        //!< Invalid use of repetition operators such as using '*' as the first character.
        ERR_BRACE = REG_EBRACE,         //!< Un-matched brace interval operators.
        ERR_BRACK = REG_EBRACK,         //!< Un-matched bracket list operators.
        ERR_COLLATE = REG_ECOLLATE,     //!< Invalid collating element.
        ERR_CTYPE = REG_ECTYPE,         //!< Unknown character class name.
        ERR_END = REG_EEND,             //!< Nonspecific error.  This is not defined by POSIX.2.
        ERR_ESCAPE = REG_EESCAPE,       //!< Trailing backslash.
        ERR_PAREN = REG_EPAREN,         //!< Un-matched parenthesis group operators.
        ERR_RANGE = REG_ERANGE,         //!< Invalid  use  of  the  range operator; for example, the ending point of the range occurs prior to the starting point.
        ERR_SIZE = REG_ESIZE,           //!< Compiled regular expression requires a pattern buffer larger than 64 kB. This is not defined by POSIX.2.
        ERR_SPACE = REG_ESPACE,         //!< The regex routines ran out of memory.
        ERR_SUBREG = REG_ESUBREG,       //!< Invalid back reference to a subexpression.
        ERR_RPAREN = REG_ERPAREN        //!< Unmatched ) or \)
    };

    /**
     * \enum RegCExtend
     * This are bits who can be used for compiling a regular expression.
     * 
     * \brief POSIX `cflags' bits (i.e., information for `regcomp').
     */
    enum RegCExtend
    {
        RC_NONE = 0,                    //!< No extension
        RC_EXTENDED = REG_EXTENDED,     //!< If this bit is set, then use extended regular expression syntax. If not set, then use basic regular expression syntax.
        RC_ICASE = REG_ICASE,           //!< If this bit is set, then ignore case when matching. If not set, then case is significant.
        RC_NOSUB = REG_NOSUB,           //!< If this bit is set, then report only success or fail in regexec. If not set, then returns differ between not matching and errors.
        RC_NEWLINE = REG_NEWLINE        //!< If this bit is set, then anchors do not match at newline characters in the string. If not set, then anchors do match at newlines.
    };

    /**
     * \enum RegEExtend
     * This are bits who can be used for the execution of a previous compiled
     * expression.
     * 
     * \brief POSIX `eflags' bits (i.e., information for `regexec').
     */
    enum RegEExtend
    {
        RE_NONE = 0,                    //!< No extension
        RE_NOTBOL = REG_NOTBOL,         //!< If this bit is set, then the beginning-of-line operator doesn't match the beginning of the string (presumably because it's not the beginning of a line). If not set, then the beginning-of-line operator does match the beginning of the string.
        RE_NOTEOL = REG_NOTEOL,         //!< Like R_NOTBOL, except for the end-of-line.
        RE_STARTEND = REG_STARTEND      //!< Use PMATCH[0] to delimit the start and end of the search in the buffer.
    };

    /**
     * \class Regexp
     * \brief Small wrapper class for the POSIX regular expression functions.
     * 
     * This is a wrapper class for the POSIX functions regcomp and regexec.
     * Following is a brief description of the valid patterns and their meaning.
     * 
     * <table>
     * <tr><th>Symbol</th><th>Description</th></tr>
     * <tr><td>*</td><td>It tells the computer to match the preceding character
     * (or set of characters) for 0 or more times (upto infinite).</td></tr>
     * <tr><td>+</td><td>It tells the computer to repeat the preceding character
     * (or set of characters) for atleast one or more times(upto infinite).</td></tr>
     * <tr><td>{...}</td><td>It tells the computer to repeat the preceding
     * character (or set of characters) for as many times as the value inside
     * this bracket.</td></tr>
     * <tr><td>.</td><td>The dot symbol can take place of any other symbol, that
     * is why it is called the wildcard character.</td></tr>
     * <tr><td>?</td><td>This symbol tells the computer that the preceding
     * character may or may not be present in the string to be matched.</td></tr>
     * <tr><td>^</td><td><i>Setting position for match</i>: tells the computer
     * that the match must start at the beginning of the string or line.</td></tr>
     * <tr><td>$</td><td>It tells the computer that the match must occur at the
     * end of the string or before \\n at the end of the line or string.</td></tr>
     * </table>
     * 
     * <b>Character classes</b><br>
     * A character class matches any one of a set of characters. It is used to
     * match the most basic element of a language like a letter, a digit, space,
     * a symbol etc.
     * 
     * <table>
     * <tr><th>Symbol</th><th>Description</th></tr>
     * <tr><td>/s</td><td>matches any whitespace characters such as space and tab</td></tr>
     * <tr><td>/S</td><td>matches any non-whitespace characters</td></tr>
     * <tr><td>/d</td><td>matches any digit character</td></tr>
     * <tr><td>/D</td><td>matches any non-digit characters</td></tr>
     * <tr><td>/w</td><td>matches any word character (basically alpha-numeric)</td></tr>
     * <tr><td>/W</td><td>matches any non-word character</td></tr>
     * <tr><td>/b</td><td>matches any word boundary (this would include spaces, dashes, commas, semi-colons, etc)</td></tr>
     * <tr><td>[set_of_characters]</td><td>Matches any single character in set_of_characters. By default, the match is case-sensitive.</td></tr>
     * <tr><td>[^set_of_characters]</td><td><i>Negation</i>: Matches any single character that is not in set_of_characters. By default, the match is case sensitive.</td></tr>
     * <tr><td>[first-last]</td><td><i>Character range</i>: Matches any single character in the range from first to last.</td></tr>
     * </table>
     */
    class Regexp
    {
        public:
            /**
             * \struct RegException
             * This class is the error class. If during the compilation or
             * execution of a regular expression an error occure, this exception
             * will be thrown.
             */
            struct RegException : public std::exception
            {
                RegException(RegError err, regex_t& r)
                {
                    e = err;
                    regex = r;
                }

                /**
                 * Retrieves the error occured and returns an error message.
                 * 
                 * @return
                 * A pointer to a 0 terminated string containing an error
                 * message.
                 */
                const char *what() const throw()
                {
                    RegException *my = const_cast<RegException *>(this);
                    regerror(e, &regex, &my->Err[0], sizeof(Err));
                    return &my->Err[0];
                }

                private:
                    RegError e;
                    regex_t regex;
                    char Err[255];
            };

            /**
             * Basic constructor.
             */
            Regexp() { valid = false; }
            /**
             * Constructor.
             * 
             * @param re
             * A pattern containing the regular expression.
             * 
             * @param ext
             * Optional: The flags of \a RegCExtend.
             */
            Regexp(const String& re, RegCExtend ext=RC_NONE);
            /**
             * Constructor.
             * 
             * @param re
             * A pattern containing the regular expression.
             * 
             * @param ext
             * Optional: The flags of \a RegCExtend.
             */
            Regexp(const std::string& re, RegCExtend ext=RC_NONE);
            /**
             * Constructor.
             * 
             * @param re
             * A pattern containing the regular expression.
             * 
             * @param ext
             * Optional: The flags of \a RegCExtend.
             */
            Regexp(const char *re, RegCExtend ext=RC_NONE);
            ~Regexp();

            /**
             * This function only sets the pattern for the regular expression.
             * It may be used later.
             * 
             * @param re
             * A pattern containing the regular expression.
             */
            void setRegexp(const String& re);
            /**
             * This function only sets the pattern for the regular expression.
             * It may be used later.
             * 
             * @param re
             * A pattern containing the regular expression.
             */
            void setRegexp(const std::string& re);
            /**
             * This function only sets the pattern for the regular expression.
             * It may be used later.
             * 
             * @param re
             * A pattern containing the regular expression.
             */
            void setRegexp(const char *re);

            /**
             * This function will work only, if the class was initialized with
             * a regular expression either when constructing the class or by
             * calling the function setRegexp().
             * 
             * @param ext
             * Optional: The flags of \a RegCExtend.
             * 
             * @return
             * TRUE if the pattern could be compiled.
             * On error it throws \a RegException.
             */
            bool compile(RegCExtend ext=RC_NONE);
            /**
             * Expects a regular expression in \b re. The function only compiles
             * the expression but does not execute it.
             * 
             * @param re
             * A pattern containing a regular expression.
             * 
             * @param ext
             * Optional: The flags of \a RegCExtend.
             * 
             * @return
             * TRUE if the pattern could be compiled.
             * On error it throws \a RegException.
             */
            bool compile(const String& re, RegCExtend ext=RC_NONE);
            /**
             * Expects a regular expression in \b re. The function only compiles
             * the expression but does not execute it.
             * 
             * @param re
             * A pattern containing a regular expression.
             * 
             * @param ext
             * Optional: The flags of \a RegCExtend.
             * 
             * @return
             * TRUE if the pattern could be compiled.
             * On error it throws \a RegException.
             */
            bool compile(const std::string& re, RegCExtend ext=RC_NONE);
            /**
             * Expects a regular expression in \b re. The function only compiles
             * the expression but does not execute it.
             * 
             * @param re
             * A pattern containing a regular expression.
             * 
             * @param ext
             * Optional: The flags of \a RegCExtend.
             * 
             * @return
             * TRUE if the pattern could be compiled.
             * On error it throws \a RegException.
             */
            bool compile(const char *re, RegCExtend ext=RC_NONE);

            /**
             * This function expects, that a pattern was set previous and could
             * be successfully compiled. This could happen with the contruction
             * of the class or by using the functions setRegexp() and/or
             * compile().
             * 
             * The result will be saved internal and can be retrieved with the
             * method getResult().
             * 
             * @param str
             * A string applied to the previous compiled pattern.
             * 
             * @param ext
             * Optional: The flags of \a RegCExtend.
             * 
             * @return
             * TRUE if the result of the execution was a <i>match</i>.
             * On error it throws \a RegException.
             */
            bool execute(const String& str, RegEExtend ext = RE_NONE);
            /**
             * This function expects, that a pattern was set previous and could
             * be successfully compiled. This could happen with the contruction
             * of the class or by using the functions setRegexp() and/or
             * compile().
             * 
             * The result will be saved internal and can be retrieved with the
             * method getResult().
             * 
             * @param str
             * A string applied to the previous compiled pattern.
             * 
             * @param ext
             * Optional: The flags of \a RegCExtend.
             * 
             * @return
             * TRUE if the result of the execution was a <i>match</i>.
             * On error it throws \a RegException.
             */
            bool execute(const std::string& str, RegEExtend ext = RE_NONE);
            /**
             * This function expects, that a pattern was set previous and could
             * be successfully compiled. This could happen with the contruction
             * of the class or by using the functions setRegexp() and/or
             * compile().
             * 
             * The result will be saved internal and can be retrieved with the
             * method getResult().
             * 
             * @param str
             * A string applied to the previous compiled pattern.
             * 
             * @param ext
             * Optional: The flags of \a RegCExtend.
             * 
             * @return
             * TRUE if the result of the execution was a <i>match</i>.
             * On error it throws \a RegException.
             */
            bool execute(const char *str, RegEExtend ext = RE_NONE);
            /**
             * This function takes a pattern \b re and a string \b str as
             * arguments. It compiles the pattern and executes it, applying
             * the string.
             * 
             * @param re
             * A pattern containing a regular expression.
             * 
             * @param str
             * A string applied to the previous compiled pattern.
             * 
             * @param ext
             * Optional: The flags of \a RegEExtend.
             * 
             * @return
             * TRUE if the result of the execution was a <i>match</i>.
             * On error it throws \a RegException.
             */
            bool execute(const String& re, const String& str, RegEExtend ext = RE_NONE);
            /**
             * This function takes a pattern \b re and a string \b str as
             * arguments. It compiles the pattern and executes it, applying
             * the string.
             * 
             * @param re
             * A pattern containing a regular expression.
             * 
             * @param str
             * A string applied to the previous compiled pattern.
             * 
             * @param ext
             * Optional: The flags of \a RegEExtend.
             * 
             * @return
             * TRUE if the result of the execution was a <i>match</i>.
             * On error it throws \a RegException.
             */
            bool execute(const std::string& re, const std::string& str, RegEExtend ext = RE_NONE);
            /**
             * This function takes a pattern \b re and a string \b str as
             * arguments. It compiles the pattern and executes it, applying
             * the string.
             * 
             * @param re
             * A pattern containing a regular expression.
             * 
             * @param str
             * A string applied to the previous compiled pattern.
             * 
             * @param ext
             * Optional: The flags of \a RegEExtend.
             * 
             * @return
             * TRUE if the result of the execution was a <i>match</i>.
             * On error it throws \a RegException.
             */
            bool execute(const char *re, const char *str, RegEExtend ext = RE_NONE);

            /**
             * This function expects, that a pattern was set previous and could
             * be successfully compiled. This could happen with the contruction
             * of the class or by using the functions setRegexp() and/or
             * compile().
             * 
             * The result will be saved internal and can be retrieved with the
             * method getResult().
             * 
             * @param str
             * A string applied to the previous compiled pattern.
             * 
             * @param cext
             * Optional: The flags of \a RegCExtend.
             * 
             * @param eext
             * Optional: The flags of \a RegEExtend.
             * 
             * @return
             * The matching string if the pattern matched, or an empty string if
             * not.
             * On error it throws \a RegException.
             */
            String match(const String& str, RegCExtend cext=RC_NONE, RegEExtend eext = RE_NONE);
            /**
             * This function expects, that a pattern was set previous and could
             * be successfully compiled. This could happen with the contruction
             * of the class or by using the functions setRegexp() and/or
             * compile().
             * 
             * The result will be saved internal and can be retrieved with the
             * method getResult().
             * 
             * @param str
             * A string applied to the previous compiled pattern.
             * 
             * @param cext
             * Optional: The flags of \a RegCExtend.
             * 
             * @param eext
             * Optional: The flags of \a RegEExtend.
             * 
             * @return
             * The matching string if the pattern matched, or an empty string if
             * not.
             * On error it throws \a RegException.
             */
            std::string match(const std::string& str, RegCExtend cext=RC_NONE, RegEExtend eext = RE_NONE);
            /**
             * This function takes a pattern \b re and a string \b str as
             * arguments. It compiles the pattern and executes it, applying
             * the string.
             * 
             * @param re
             * A pattern containing a regular expression.
             * 
             * @param str
             * A string applied to the previous compiled pattern.
             * 
             * @param cext
             * Optional: The flags of \a RegCExtend.
             * 
             * @param eext
             * Optional: The flags of \a RegEExtend.
             * 
             * @return
             * The matching string if the pattern matched, or an empty string if
             * not.
             * On error it throws \a RegException.
             */
            String match(const String& re, const String& str, RegCExtend cext=RC_NONE, RegEExtend eext = RE_NONE);
            /**
             * This function takes a pattern \b re and a string \b str as
             * arguments. It compiles the pattern and executes it, applying
             * the string.
             * 
             * @param re
             * A pattern containing a regular expression.
             * 
             * @param str
             * A string applied to the previous compiled pattern.
             * 
             * @param cext
             * Optional: The flags of \a RegCExtend.
             * 
             * @param eext
             * Optional: The flags of \a RegEExtend.
             * 
             * @return
             * The matching string if the pattern matched, or an empty string if
             * not.
             * On error it throws \a RegException.
             */
            std::string match(const std::string& re, const std::string& str, RegCExtend cext=RC_NONE, RegEExtend eext = RE_NONE);

            /**
             * This function expects, that a pattern was set previous and could
             * be successfully compiled. This could happen with the contruction
             * of the class or by using the functions setRegexp() and/or
             * compile().
             * 
             * The result will be saved internal and can be retrieved with the
             * method getResult().
             * 
             * @param str
             * A string applied to the previous compiled pattern.
             * 
             * @param cext
             * Optional: The flags of \a RegCExtend.
             * 
             * @param eext
             * Optional: The flags of \a RegEExtend.
             * 
             * @return
             * A pointer to self.
             * On error it throws \a RegException.
             */
            Regexp& getResult(const String& str, RegCExtend cext=RC_NONE, RegEExtend eext=RE_NONE);
            /**
             * This function takes a pattern \b re and a string \b str as
             * arguments. It compiles the pattern and executes it, applying
             * the string. The result is stored intern and can be retrieved
             * with the method getResult().
             * 
             * @param re
             * A pattern containing a regular expression.
             * 
             * @param str
             * A string applied to the previous compiled pattern.
             * 
             * @param cext
             * Optional: The flags of \a RegCExtend.
             * 
             * @param eext
             * Optional: The flags of \a RegEExtend.
             * 
             * @return
             * A pointer to self.
             * On error it throws \a RegException.
             */
            Regexp& getResult(const String& re, const String& str, RegCExtend cext=RC_NONE, RegEExtend eext=RE_NONE);
            /**
             * Returns a previous successfully executed regular expression as a
             * \a String.
             * 
             * @return
             * A \a String containing the reult of a successfully executed
             * regular expression.
             */
            String& getResult();
            /**
             * Returns a previous successfully executed regular expression as a
             * \a String.
             * 
             * @return
             * A \a String containing the reult of a successfully executed
             * regular expression.
             */
            String& getResult() const;

        private:
            RegError _compile(const char *re, RegCExtend ext);
            String _assignMatch(const String& str, regmatch_t match[]);

            String mRegstr;
            String mResult;
            regex_t regex;
            bool valid;
            bool exec;
    };
}

#endif

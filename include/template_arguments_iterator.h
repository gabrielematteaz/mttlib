#ifndef MTTLIB_INCLUDE_TEMPLATE_ARGUMENTS_ITERATOR_H_
#define MTTLIB_INCLUDE_TEMPLATE_ARGUMENTS_ITERATOR_H_

#include <cassert>
#include <optional>
#include <utility>

#include "template_characters_span.h"
#include "template_string.h"

namespace mttlib {
  enum class ArgumentsIteratorResult {
    kSuccess,
    kNoArgumentFound,
    kInvalidArgumentFound,
    kMemoryAllocationFailure
  };

  template < typename CharacterType >
  class TemplateArgumentsIterator {
    using StringType = TemplateString < CharacterType >;
    using CharactersSpanType = TemplateCharactersSpan < CharacterType >;

    static constexpr CharacterType kCharacterTabulation = 0x09;
    static constexpr CharacterType kSpace = 0x20;
    static constexpr CharacterType kQuotationMark = 0x22;
    static constexpr CharacterType kApostrophe = 0x27;
    static constexpr CharacterType kReverseSolidus = 0x5C;

    StringType command_line_;
    StringType argument_;
    int offset_;

    TemplateArgumentsIterator(StringType &&command_line) noexcept :
      command_line_(std::move(command_line)),
      argument_(),
      offset_(0)
    {

    }

  public:
    TemplateArgumentsIterator(const TemplateArgumentsIterator &) = delete;
    TemplateArgumentsIterator(TemplateArgumentsIterator &&) = default;
    TemplateArgumentsIterator &operator = (const TemplateArgumentsIterator &) = delete;
    TemplateArgumentsIterator &operator = (TemplateArgumentsIterator &&) = default;

    // assert(command_line != nullptr)
    static std::optional < TemplateArgumentsIterator > Construct(
        const CharacterType *command_line) noexcept {
      assert(command_line != nullptr);

      std::optional < StringType > command_line_copy =
          StringType::Construct(command_line, true);

      if (command_line_copy.has_value() == false) {
        return std::nullopt;
      }

      return TemplateArgumentsIterator(std::move(*command_line_copy));
    }

    static std::optional < TemplateArgumentsIterator > Construct(
        const StringType &command_line) noexcept {
      std::optional < StringType > command_line_copy =
          StringType::Construct(command_line, true);

      if (command_line_copy.has_value() == false) {
        return std::nullopt;
      }

      return TemplateArgumentsIterator(std::move(*command_line_copy));
    }

    const StringType &command_line() const noexcept {
      return command_line_;
    }

    const StringType &argument() const noexcept {
      return argument_;
    }

    int offset() const noexcept {
      return offset_;
    }

    ArgumentsIteratorResult Increment() noexcept {
      int remainder = command_line_.length() - offset_;
      const CharacterType *first = command_line_.buffer() + offset_;

      for (; remainder != 0; --remainder, ++first) {
        if (*first != kSpace && *first != kCharacterTabulation) {
          break;
        }
      }

      if (remainder == 0) {
        return ArgumentsIteratorResult::kNoArgumentFound;
      }

      const CharacterType *last = first;
      bool skip = false;
      bool within_quotation_marks = false;
      bool within_apostrophes = false;

      for (; remainder != 0; --remainder, ++last) {
        if (skip) {
          skip = false;
        }
        else if (*last == kQuotationMark) {
          if (within_apostrophes == false) {
            within_quotation_marks = !within_quotation_marks;
          }
        }
        else if (*last == kReverseSolidus) {
          if (within_apostrophes == false) {
            skip = true;
          }
        }
        else if (*last == kApostrophe) {
          if (within_quotation_marks == false) {
            within_apostrophes = !within_apostrophes;
          }
        }
        else if (*last == kSpace || *last == kCharacterTabulation) {
          if (within_quotation_marks == false && within_apostrophes == false) {
            break;
          }
        }
      }

      if (within_quotation_marks || within_apostrophes) {
        return ArgumentsIteratorResult::kInvalidArgumentFound;
      }

      int worst_case_needed_memory = static_cast < int > (last - first);

      if (argument_.Reserve(worst_case_needed_memory) == false) {
        return ArgumentsIteratorResult::kMemoryAllocationFailure;
      }

      argument_.Clear();
      offset_ = command_line_.length() - remainder;
      skip = false;

      const CharacterType *current = first;
      bool update = false;

      for (; current != last; ++current) {
        if (skip) {
          if (*current == kQuotationMark || *current == kReverseSolidus ||
              within_quotation_marks == false) {
            argument_.Append(first, current - 1);
            first = current;
          }

          skip = false;
        }
        else if (*current == kQuotationMark) {
          if (within_apostrophes == false) {
            within_quotation_marks = !within_quotation_marks;
            update = true;
          }
        }
        else if (*current == kReverseSolidus) {
          if (within_apostrophes == false) {
            skip = true;
          }
        }
        else if (*current == kApostrophe) {
          if (within_quotation_marks == false) {
            within_apostrophes = !within_apostrophes;
            update = true;
          }
        }

        if (update) {
          argument_.Append(first, current);
          first = current + 1;
          update = false;
        }
      }

      argument_.Append(first, current - skip);

      return ArgumentsIteratorResult::kSuccess;
    }
  };

  template < >
  constexpr char TemplateArgumentsIterator < char >::kCharacterTabulation = '\t';

  template < >
  constexpr char TemplateArgumentsIterator < char >::kSpace = ' ';

  template < >
  constexpr char TemplateArgumentsIterator < char >::kQuotationMark = '\"';

  template < >
  constexpr char TemplateArgumentsIterator < char >::kApostrophe = '\'';

  template < >
  constexpr char TemplateArgumentsIterator < char >::kReverseSolidus = '\\';

  template < >
  constexpr wchar_t TemplateArgumentsIterator < wchar_t >::kCharacterTabulation = L'\t';

  template < >
  constexpr wchar_t TemplateArgumentsIterator < wchar_t >::kSpace = L' ';

  template < >
  constexpr wchar_t TemplateArgumentsIterator < wchar_t >::kQuotationMark = L'\"';

  template < >
  constexpr wchar_t TemplateArgumentsIterator < wchar_t >::kApostrophe = L'\'';

  template < >
  constexpr wchar_t TemplateArgumentsIterator < wchar_t >::kReverseSolidus = L'\\';

  using ArgumentsIterator = TemplateArgumentsIterator < char >;
  using WideArgumentsIterator = TemplateArgumentsIterator < wchar_t >;
}

#endif
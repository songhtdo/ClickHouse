#pragma once

#include <Core/Block.h>
#include <Processors/Formats/IInputFormat.h>
#include <Processors/Formats/IRowInputFormat.h>
#include <Formats/FormatSettings.h>
#include <Processors/Formats/Impl/ConstantExpressionTemplate.h>

#include <IO/PeekableReadBuffer.h>
#include <Parsers/ExpressionListParsers.h>

namespace DB
{

class Context;
class ReadBuffer;


/** Stream to read data in VALUES format (as in INSERT query).
  */
class ValuesBlockInputFormat : public IInputFormat
{
public:
    /** Data is parsed using fast, streaming parser.
      * If interpret_expressions is true, it will, in addition, try to use SQL parser and interpreter
      *  in case when streaming parser could not parse field (this is very slow).
      * If deduce_templates_of_expressions is true, try to deduce template of expression in some row and use it
      * to parse and interpret expressions in other rows (in most cases it's faster
      * than interpreting expressions in each row separately, but it's still slower than streaming parsing)
      */
    ValuesBlockInputFormat(ReadBuffer & in_, const Block & header_, const RowInputFormatParams & params_,
                           const Context & context_, const FormatSettings & format_settings_);

    String getName() const override { return "ValuesBlockInputFormat"; }

private:
    enum class ParserType
    {
        Streaming,
        BatchTemplate,
        SingleExpressionEvaluation
    };

    typedef std::vector<std::optional<ConstantExpressionTemplate>> ConstantExpressionTemplates;

    Chunk generate() override;

    void readRow(MutableColumns & columns);

    void tryParseExpressionUsingTemplate(MutableColumnPtr & column, size_t column_idx);
    ALWAYS_INLINE inline void tryReadValue(IColumn & column, size_t column_idx);
    void parseExpression(IColumn & column, size_t column_idx);

    ALWAYS_INLINE inline void assertDelimiterAfterValue(size_t column_idx);
    ALWAYS_INLINE inline bool checkDelimiterAfterValue(size_t column_idx);

    bool shouldDeduceNewTemplate(size_t column_idx);

    void readSuffix();

    bool skipToNextRow(size_t min_chunk_size = 0, int balance = 0);

private:
    PeekableReadBuffer buf;

    RowInputFormatParams params;

    std::unique_ptr<Context> context;   /// pimpl
    const FormatSettings format_settings;

    size_t num_columns;
    size_t total_rows = 0;

    std::vector<ParserType> parser_type_for_column;
    std::vector<size_t> attempts_to_deduce_template;
    std::vector<size_t> attempts_to_deduce_template_cached;
    std::vector<size_t> rows_parsed_using_template;

    ParserExpression parser;
    ConstantExpressionTemplates templates;
    ConstantExpressionTemplate::Cache templates_cache;

    DataTypes types;
};

}

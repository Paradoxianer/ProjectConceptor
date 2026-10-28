#ifndef ATTRIBUTE_TEXT_H
#define ATTRIBUTE_TEXT_H

#include <app/Message.h>
#include <support/String.h>

/** The value of an attribute block {Name, Value, ...} in Node::Data as
 * plain text, the same for every kind (#146): text as it is, yes/no as
 * "true"/"false", a number with its unit ("25.5 €"), a color as #rrggbb,
 * a date as YYYY-MM-DD. For exports and search; the editor shows them
 * its own way. B_BAD_VALUE if the block has no Value. */
status_t	AttributeValueText(const BMessage &attribute, BString *text);

/** a number as short as it gets: no trailing zeros, '.' as the point */
BString		NumberText(float number);

/** "25,5 €", "1,234.5 km", "-3h": the number at the start and the rest as
 * its unit. With both '.' and ',' the last one is the decimal point, one
 * alone always is; spaces and apostrophes group digits. B_BAD_VALUE if
 * the text doesn't start with a number. */
status_t	ParseNumberWithUnit(const char *text, float *number, BString *unit);

#endif

#ifndef ATTRIBUTE_TEXT_H
#define ATTRIBUTE_TEXT_H

#include <app/Message.h>
#include <interface/GraphicsDefs.h>
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

/** the word for an attribute kind (its what) in macros and icon names:
 * "text", "bool", "note", "number", "choice", "date", "color", "link",
 * "file", "image"; NULL for a what that is no attribute kind */
const char*	AttributeKindName(uint32 what);

/** the what of a kind word; false for an unknown one */
bool		AttributeKindFromName(const char *name, uint32 *what);

/** adds the value a new attribute of attribute->what starts with: a color
 * gets accent, a choice list the lines of choices and the first of them
 * as its value, a date today */
void		AddAttributeDefaultValue(BMessage *attribute, rgb_color accent,
				const char *choices);

#endif

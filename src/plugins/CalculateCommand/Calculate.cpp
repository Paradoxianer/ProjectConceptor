#include "Calculate.h"
#include "PCommandManager.h"
#include "ProjectConceptorDefs.h"

#include <math.h>
#include <support/Debug.h>


Calculate::Calculate():PCommand()
{
}

static const property_info kCalculateProperties[] = {
	{ "Calculate", { B_EXECUTE_PROPERTY, 0 }, { B_DIRECT_SPECIFIER, 0 },
		"Arithmetic on macro variables - stores \"left <operator> right\" "
		"under \"resultVariable\" in the macro's value context. operator: "
		"+ - * / mod min max (binary, both operands used) or round floor "
		"ceil abs (unary, \"right\" ignored) - restore the result later "
		"with e.g. dx=$<resultVariable>.",
		0, {0},
		{ { { {"left", B_FLOAT_TYPE}, {"operator", B_STRING_TYPE},
			  {"right", B_FLOAT_TYPE}, {"resultVariable", B_STRING_TYPE} } } } },
};

const property_info* Calculate::PropertyInfo(int32 *count)
{
	*count	= 1;
	return kCalculateProperties;
}

BMessage* Calculate::Do(PDocument *doc, BMessage *settings)
{
	float	left	= 0.0f;
	float	right	= 0.0f;
	BString	op;
	BString	resultVariable;
	settings->FindFloat("left",&left);
	settings->FindFloat("right",&right);
	settings->FindString("operator",&op);
	settings->FindString("resultVariable",&resultVariable);

	float	result		= 0.0f;
	bool	recognized	= true;
	if (op == "+")				result	= left+right;
	else if (op == "-")		result	= left-right;
	else if (op == "*")		result	= left*right;
	else if (op == "/")		result	= left/right;
	else if (op == "mod")		result	= fmodf(left,right);
	else if (op == "min")		result	= (left < right) ? left : right;
	else if (op == "max")		result	= (left > right) ? left : right;
	else if (op == "round")	result	= roundf(left);
	else if (op == "floor")	result	= floorf(left);
	else if (op == "ceil")		result	= ceilf(left);
	else if (op == "abs")		result	= fabsf(left);
	else						recognized	= false;

	// no silent fallback (project convention) - an unrecognized operator
	// (a typo, most likely) leaves resultVariable genuinely untouched
	// rather than quietly storing 0.0 as if that were a real answer
	if (!recognized) {
		PRINT(("Calculate::Do - unknown operator \"%s\"\n",op.String()));
	} else if ((resultVariable.Length() > 0) && (manager->GetValueContext() != NULL)) {
		manager->GetValueContext()->RemoveName(resultVariable.String());
		manager->GetValueContext()->AddFloat(resultVariable.String(),result);
	}

	settings	= PCommand::Do(doc,settings);
	doc->SetModified();
	return settings;
}

void Calculate::Undo(PDocument *doc,BMessage *undo)
{
	// nothing to do :) - see the class comment
	doc->SetModified();
}



void Calculate::AttachedToManager(void)
{
}

void Calculate::DetachedFromManager(void)
{
}

#include "variant_db.h"

LocalVector<VariantExtensionType*> VariantDB::extensions;
HashMap<StringName, Variant::Type> VariantDB::type_by_name;

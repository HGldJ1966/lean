/*
Copyright (c) 2014 Microsoft Corporation. All rights reserved.
Released under Apache 2.0 license as described in the file LICENSE.

Author: Leonardo de Moura
*/
#include "kernel/environment.h"
#include "kernel/instantiate.h"
#include "kernel/type_checker.h"
#include "library/module.h"
#include "library/protected.h"

namespace lean {
environment mk_induction_on(environment const & env, name const & n) {
    if (!env.impredicative())
        throw exception("induction_on generation failed, Prop/Type.{0} is not impredicative in the given environment");
    name rec_on_name(n, "rec_on");
    name induction_on_name(n, "induction_on");
    name_generator ngen;
    declaration rec_on_decl   = env.get(rec_on_name);
    declaration ind_decl      = env.get(n);
    unsigned rec_on_num_univs = length(rec_on_decl.get_univ_params());
    unsigned ind_num_univs    = length(ind_decl.get_univ_params());
    bool opaque               = false;
    bool use_conv_opt         = true;
    environment new_env       = env;
    if (rec_on_num_univs == ind_num_univs) {
        // easy case, induction_on is just an alias for rec_on
        certified_declaration cdecl = check(new_env,
                                            mk_definition(new_env, induction_on_name, rec_on_decl.get_univ_params(),
                                                          rec_on_decl.get_type(), rec_on_decl.get_value(),
                                                          opaque, rec_on_decl.get_module_idx(), use_conv_opt));
        new_env = module::add(new_env, cdecl);
    } else {
        level_param_names induction_on_univs = tail(rec_on_decl.get_univ_params());
        level_param_names from = to_list(head(rec_on_decl.get_univ_params()));
        levels            to   = to_list(mk_level_zero());
        expr induction_on_type  = instantiate_univ_params(rec_on_decl.get_type(), from, to);
        expr induction_on_value = instantiate_univ_params(rec_on_decl.get_value(), from, to);
        certified_declaration cdecl = check(new_env,
                                            mk_definition(new_env, induction_on_name, induction_on_univs,
                                                          induction_on_type, induction_on_value,
                                                          opaque, rec_on_decl.get_module_idx(), use_conv_opt));
        new_env = module::add(new_env, cdecl);
    }
    return add_protected(new_env, induction_on_name);
}
}

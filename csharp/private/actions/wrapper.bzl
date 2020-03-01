def write_wrapper_main_cc(ctx, name, template, dotnetexe, argv1 = None, argv2 = None):
    main_cc = ctx.actions.declare_file("%s.main.cc" % name)

    dotnetexe_path = dotnetexe.short_path[3:] # eh?

    ctx.actions.expand_template(
        template = template,
        output = main_cc,
        substitutions = {
            "{DotnetExe}": dotnetexe_path,
            "{Argv1}": argv1 or "",
            "{Argv2}": argv2 or "",
        },
    )

    return main_cc

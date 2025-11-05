# из корня
ln -sf build/compile_commands.json compile_commands.json
ln -sf ../build/compile_commands.json cli/compile_commands.json
ln -sf ../build/compile_commands.json lockfile/compile_commands.json
# ...по аналогии для других подпроектов


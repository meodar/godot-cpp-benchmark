use godot::{init::ExtensionLibrary, prelude::gdextension};

mod perf_monitor;
mod random_table;

struct BenchmarkCppRsExt;

#[gdextension]
unsafe impl ExtensionLibrary for BenchmarkCppRsExt {}

use std::env;
use std::fs::create_dir;
use std::path::Path;

extern crate cc;

fn main() {
    let manifest_dir = env::var("CARGO_MANIFEST_DIR").unwrap();
    let cargo_dir =  Path::new(manifest_dir.as_str());
    let lib_dir = cargo_dir.join("lib");
    let variadic_c = cargo_dir.join("../variadic.c");

    if !lib_dir.is_dir() {
        create_dir(&lib_dir).unwrap();
    }

    cc::Build::new()
        .flag("-c")
        .file(variadic_c)
        .flag("-fPIC")
        .flag("-w") // Hide warnings; cc will pass them to cargo annoyingly
        .out_dir(&lib_dir)
        .compile("variadic");

    println!("cargo:rustc-link-search=native={}", lib_dir.display());
}

let
    pkgs = import (fetchTarball "https://github.com/NixOS/nixpkgs/archive/cf8cc1201be8bc71b7cbbbdaf349b22f4f99c7ae.tar.gz") {};
in pkgs.mkShell {
    packages = [
        (pkgs.python3.withPackages (python-pkgs: [
            python-pkgs.pandas
            python-pkgs.numpy
            python-pkgs.tqdm
            python-pkgs.networkx
        ]))
    ];
}


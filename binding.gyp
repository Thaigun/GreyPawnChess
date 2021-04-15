{
    "targets": 
    [
        {
            "target_name": "greypawnchess",
            "sources": ["src/chessAddon.cpp"],
            'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
            'include_dirs': ["<!(node -p \"require('node-addon-api').include_dir\")"]
        }
    ]
}
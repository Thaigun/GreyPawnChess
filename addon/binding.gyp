{
    "targets": 
    [
        {
            "target_name": "greypawnchess",
            "sources": [
                "<!@(node -p \"require('fs').readdirSync('./engine/src').filter(f=>f.endsWith('.cpp') || f.endsWith('.cc')).map(f=>'./engine/src/'+f).join(' ')\")",
                "<!@(node -p \"require('fs').readdirSync('./napi').filter(f=>f.endsWith('.cpp') || f.endsWith('.cc')).map(f=>'./napi/'+f).join(' ')\")"
            ],
            'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
            'include_dirs': ["<!(node -p \"require('node-addon-api').include_dir\")"] 
        }
    ]
}
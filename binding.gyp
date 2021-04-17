{
    "targets": 
    [
        {
            "target_name": "greypawnchess",
            "sources": [
                "<!@(node -p \"require('fs').readdirSync('./src').filter(f=>f.endsWith('.cpp') || f.endsWith('.cc')).map(f=>'src/'+f).join(' ')\")"
            ],
            'defines': [ 'NAPI_DISABLE_CPP_EXCEPTIONS' ],
            'include_dirs': ["<!(node -p \"require('node-addon-api').include_dir\")"] 
        }
    ]
}
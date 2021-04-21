const path = require('path');

module.exports = {
    mode: 'production',
    
    context: path.resolve(__dirname, './static'),
    
    entry: {
        index: {
            import: './app.js',
            filename: 'index.js',
        },
    },
    
    output: {
        path: path.resolve(__dirname, '../public'),
    },

    optimization: {
        splitChunks: {
            chunks: 'all',
            name: (entrypoint) => `${entrypoint.name}`,
        },
        runtimeChunk: {
            name: (entrypoint) => `runtime-${entrypoint.name}`,
        }
    },

    module: {
        rules: [
            {
                test: /\.css$/,
                use: [
                    'style-loader',
                    'css-loader',
                ],
            },
            {
                test: /\.(html|xml|eot|woff|ttf|svg|gif|png|jpg|bmp|ico)$/i,
                loader: 'file-loader',
                options: {
                    name: '[path][name].[ext]',
                }, 
            }
        ],
    },
};

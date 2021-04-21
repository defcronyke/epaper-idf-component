const path = require('path');

module.exports = {
    devServer: {
        watchContentBase: true,
        hot: true,
    },
    
    entry: {
        index: {
            import: './app.js',
        },
    },
    
    output: {
        filename: 'index.js',
        path: path.resolve(__dirname, '../public'),
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

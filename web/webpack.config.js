// const glob = require("glob");
const path = require('path');

module.exports = {
    devServer: {
        historyApiFallback: {
            disableDotRule: true,
        },
        watchContentBase: true,
        hot: true,
    },
    
    entry: {
        index: {
            import: './app.js',
            filename: 'index.js',
        },
        // gen_204: {
        //     import: './gen_204',
        // },
    },
    
    output: {
        path: path.resolve(__dirname, '../public'),
        // filename: 'index.js',
    },

    module: {
        rules: [
            {
                test: /\.(css)/,
                use: [
                    'style-loader',
                    'css-loader',
                ],
            },
            {
                test: /\.(html|xml|eot|woff|ttf|svg|gif|png|jpg|bmp|ico)/,
                loader: 'file-loader',
                options: {
                    name: '[path][name].[ext]',
                },
            },
            {
                test: /(gen_204)/,
                loader: 'file-loader',
                options: {
                    name: '[path][name]',
                },
            },
        ],
    },
};

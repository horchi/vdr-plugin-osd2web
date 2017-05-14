// hier wird beim Entwickeln der code hingepublisht, muss ggf. angepasst werden    
const outPathDev = '/var/lib/vdr/plugins/osd2web/http/dev';
// Wenn ein neuer Skin hinzugefügt werden soll, 
// einfach das Object um den entsprechenden Ordnernamen erweitern und dann die verfügbaren themes eintragen:

const skins = { 
  'default': ['default'], 
  //'skin-example': ['default', 'theme-example'] 
};

/*============================================================================ */

const {
   resolve
} = require('path')
const isDev = (process.env.NODE_ENV === 'dev');

const webpack = require('webpack')
const HtmlWebpackPlugin = require('html-webpack-plugin')
const webpackUglifyJsPlugin = require('webpack-uglify-js-plugin');
const webpackMerge = require('webpack-merge');
const ExtractTextPlugin = require('extract-text-webpack-plugin');

var baseConf = {
  plugins: [/*
  new webpackUglifyJsPlugin({
    cacheFolder: resolve(__dirname, 'public/cached_uglify/'),
    debug: true,
    minimize: !true,
    sourceMap: false,
    output: {
      comments: false
    },
    compressor: {
      warnings: false
    }
  })*/
    new ExtractTextPlugin({ filename: '[name].css', allChunks: false })
  ],
  output: {
    path: isDev ? outPathDev : resolve(__dirname, 'dist'),
    filename: '[name]',
    publicPath: ''
  },
  module: {
    rules: [{
      test: /\.vue$/,
      use: ['vue-loader']
    },
    {
      test: /\.js$/,
      use: ['babel-loader'],
      exclude: /node_modules/
    },
    {
      test: /\.scss$/,
      use: ExtractTextPlugin.extract({ use: ['css-loader?sourceMap', 'postcss-loader?sourceMap', 'sass-loader?sourceMap'] })
    },
    {
      test: /\.(png|jpg|jpeg|gif|ico|eot|ttf|woff|woff2|svg|svgz)(\?.+)?$/,
      use: [{
        loader: 'url-loader',
        options: {
          limit: 10000,
          name: '[name].[ext]'
        }
      }]
    }
    ]
  },
  externals: {
    'common': 'common'
  },
  resolve: {
    extensions: [".vue", ".js", ".json", '.scss']
  },
  devtool: isDev ? '#eval-source-map' : '#source-map'
}
let skinTargets = [webpackMerge(baseConf, {
  entry: { 
    'common.js': './src/common.js'
  },
  output: { library: 'common' },
  resolve: {
      modules: [resolve(__dirname, 'node_modules')]
  },
  plugins: [
    new HtmlWebpackPlugin({
      filename: 'index.html',
      template: 'src/index.html',
      inject: false
    })
  ]
})];

for (target in skins) {
  let entries = {
    'skin.js': './src/skins/' + target + '/main.js'
  };
  let themes= skins[target];
  for (i in themes)
    entries['themes/' + themes[i]]= themes[i] + '.scss';
  skinTargets.push(webpackMerge(baseConf, {
    entry: entries,
    output: {
      path: baseConf.output.path + '/skins/' + target
    },
    resolve: {
      modules: [resolve(__dirname, 'src/skins/' + target), resolve(__dirname, 'src/components'), resolve(__dirname, 'src/themes'), resolve(__dirname, 'node_modules')]
    },
    plugins: [
      new HtmlWebpackPlugin({
        filename: 'index.html',
        chunks: ['skin'],
        template: 'src/skin.html',
        inject: false
      })
    ]
  }));

}

module.exports = skinTargets;

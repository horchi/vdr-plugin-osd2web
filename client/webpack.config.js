const { resolve } = require('path')
// hier wird beim Builden (production) der code hingepublisht und wird dann per make kopiert
var targetFolder = resolve(__dirname, 'dist');
// hier wird beim Entwickeln der code hingepublisht, muss ggf. angepasst werden
var targetFolder_dev = '/var/lib/vdr/plugins/osd2web/http/dev';
// Wenn ein neuer Skin in dieser Branch hinzugefügt werden soll,
// einfach das Object um den entsprechenden Ordnernamen erweitern und dann die verfügbaren themes eintragen:
var skins = {
  'default': ['default'],
  'horchiTft': ['plain', 'blue']
};

// Eine vorhandene skins.config.js kann Einstellungen überschreiben
var custom_conf = null;
try {
  let custom_conf = require('./skins.config.js');
  if (custom_conf.skins)
    skins = custom_conf.skins;
  else if (custom_conf.additional_skins) {
    for (target in custom_conf.additional_skins)
      skins[target] = custom_conf.additional_skins[target];
  }
  if (custom_conf.targetFolder)
    targetFolder = custom_conf.targetFolder;
  if (custom_conf.targetFolder_dev)
    targetFolder_dev = custom_conf.targetFolder_dev;
} catch (e) { };


const skin = process.env.npm_config_skin || null;
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
    path: isDev ? targetFolder_dev : targetFolder,
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

const common_conf = webpackMerge(baseConf, {
  entry: {
    'common.js': './src/common.js'
  },
  output: {
    library: 'common',
    publicPath: isDev ? '/dev/' : '/',
    chunkFilename: '[name].js'
  },
  resolve: {
    modules: [resolve(__dirname, 'node_modules')]
  },
  plugins: [
    new HtmlWebpackPlugin({
      filename: 'index.html',
      template: 'src/index.html',
      favicon: 'src/assets/favicon.ico',
      inject: false
    })
  ]
});
var conf;

if (skin){
  conf = (process.env.npm_config_common ? [common_conf,getSkinConfig(skin)] : getSkinConfig(skin));
}else {
  conf = [common_conf];
  for (target in skins) {
    conf.push(getSkinConfig(target));
  }
}


function getSkinConfig(target) {
  let entries = {
    'skin.js': './src/skins/' + target + '/main.js'
  };
  let themes = skins[target];
  for (i in themes)
    entries['themes/' + themes[i]] = themes[i] + '.scss';
  return webpackMerge(baseConf, {
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
  });
}

module.exports = conf;

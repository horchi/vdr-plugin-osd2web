const { resolve } = require('path')
// hier wird beim Builden (production) der code hingepublisht und wird dann per make kopiert
var targetFolder = resolve(__dirname, 'dist');
// hier wird beim Entwickeln der code hingepublisht, muss ggf. angepasst werden
var targetFolder_dev = '/var/lib/vdr/plugins/osd2web/http/dev';
// Wenn ein neuer Skin in dieser Branch hinzugefügt werden soll,
// einfach das Object um den entsprechenden Ordnernamen erweitern und dann die verfügbaren themes eintragen:
var buildInSkins = {
  'default': {
    themes: ['default']
  },
  'horchiTft': {
      themes: ['plain', 'blue', 'bluecd', 'gray', 'graycd']
  }
};
var customSkins= {};

const fs = require('fs');
fs.readdirSync('./src/custom/').filter(function(skinName) {
  try {

    if ('config.js' == skinName){
        // Eine vorhandene skins.config.js kann Einstellungen überschreiben
          let custom_conf = require('./src/custom/' + skinName);
          if (custom_conf.targetFolder)
            targetFolder = custom_conf.targetFolder;
          if (custom_conf.targetFolder_dev)
            targetFolder_dev = custom_conf.targetFolder_dev;
    } else if (buildInSkins[skinName]){
        let themes= buildInSkins[skinName].customThemes || ( buildInSkins[skinName].customThemes= []);
        fs.readdirSync('./src/custom/' + skinName).filter(function(file) {
          if (/\.scss$/.test(file))
            themes.push(file.slice(0,-5));
        });
    } else {
        let themes= [];
        fs.readdirSync('./src/custom/' + skinName).filter(function(file) {
          if (/\.scss$/.test(file)){
            themes.push(file.slice(0,-5));
          } else if ('main.js' == file){
            customSkins[skinName]= {customThemes:themes};
          }
        });
        if (!themes.length){
          delete customSkins[skinName];
          console.error("no theme found in " + skinName);
        }
    }
  } catch (e) {
    console.error(e);
  };
});


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
  for (target in buildInSkins) {
    conf.push(getSkinConfig(target));
  }
  for (target in customSkins) {
    conf.push(getSkinConfig(target));
  }
}

function getSkinConfig(target) {
  let skin= buildInSkins[target];
  let skinPath= 'src/skins/';
  if (!skin){
      skin= customSkins[target];
      if (!skin)
         return !console.error('Could not find skin ' + target);
      skinPath= 'src/custom/';
  }
  let entries = {
    'skin.js': './' + skinPath + target + '/main.js'
  };
  if (skin.themes)
    for (i in skin.themes)
      entries['themes/' + skin.themes[i]] = './src/skins/' + target + '/' + skin.themes[i] + '.scss';
  if (skin.customThemes)
    for (i in skin.customThemes)
      entries['themes/' + skin.customThemes[i]] = './src/custom/' + target + '/' + skin.customThemes[i] + '.scss';
  return webpackMerge(baseConf, {
    entry: entries,
    output: {
      path: baseConf.output.path + '/skins/' + target
    },
    resolve: {
      modules: [resolve(__dirname, skinPath + target), resolve(__dirname, 'src/components'), resolve(__dirname, 'node_modules')]
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

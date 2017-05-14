const autoprefixerBrowsers = require('bootstrap/grunt/postcss').autoprefixer.browsers;
module.exports = {
  plugins: [
    require('postcss-flexbugs-fixes'),
    require('autoprefixer')({ browsers: autoprefixerBrowsers })
  ]
}
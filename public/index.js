(()=>{var e={10:()=>{!function(){function e(n,t){0==n||n||(n=4e3),t||(t=1);var o=n/1e3;console.log("Reconnect requested. Will attempt reconnect in "+o+" seconds."),window.setTimeout((function(){console.log("Attempting reconnect with "+o+" second timeout.\nAttempt #: "+t),fetch(window.location.href).then((function(){var e=window.location.href+"?restarted="+t;window.location.href=e})).catch((function(o){console.log(o),window.setTimeout((function(){t++,e(n,t)}),n)}))}),n)}function n(){if(confirm("Would you like to restart the device?")){fetch("/api/util/restart",{method:"POST"}).then((function(e){return 404===e.status?{msg:"DEMO MODE: no-op"}:e.json()})).then((function(e){return console.log("POST /api/util/restart response:"),console.log(e),e})).catch((function(e){return console.log("error: unexpected response from server:"),console.log(e),e})),e(4e3);var n=document.getElementById("util-status");console.log("The device is restarting. This page will refresh when the device is ready. Please wait..."),n.innerText="The device is restarting. This page will refresh when the device is ready. Please wait..."}}console.log("js loaded"),function(){if(console.log("initializing status areas..."),window.location.href.match(/([\?\&](restarted))+/)){var e=window.location.href.replace(/(.+[\?\&](restarted\=))/,"");const t=window.location.pathname.replace(/([\?\&](restarted))+/,"");window.history.replaceState({},document.title,t);const o=document.getElementById("util-status");var n="The device has restarted and is now ready to use after "+e+" ready check";e>1&&(n+="s"),n+=".",console.log(n),o.innerText=n}}(),function(){console.log("initializing buttons...");var e=document.getElementById("restart");e&&e.addEventListener("click",(function(){n()})),console.log("buttons initialized")}()}()},314:(e,n,t)=>{"use strict";t.d(n,{Z:()=>u});var o=t(645),r=t.n(o),i=t(667),a=t.n(i),c=t(545),s=r()((function(e){return e[1]}));s.push([e.id,"@import url(https://fonts.googleapis.com/css2?family=Nanum+Myeongjo:wght@400;700;800&display=swap);"]);var l=a()(c.Z);s.push([e.id,'/* @font-face {\n    font-family: "Nanum Myeongjo";\n    src: url("./NanumMyeongjo-Regular.ttf");\n} */\n\nbody {    \n    background-color: #979633;\n}\n\n#main::after {\n    content: "";\n    background-image: url('+l+");\n    background-size: 100vw 100vh;\n    opacity: 0.6;\n    position: absolute;\n    top: 0;\n    bottom: 0;\n    left: 0;\n    right: 0;\n    z-index: -1;\n}\n\n#main {\n    font-family: 'Nanum Myeongjo', serif;\n}\n\n#main .title hr {\n    margin-top: 0.1em;\n    margin-bottom: 0.1em;\n}\n\n#main h1.title {\n    font-size: 3em;\n    background-color: rgba(200, 200, 200, 0.9);\n    padding: 0.2em;\n    padding-left: 0.6em;\n}\n\n#main h3.title {\n    font-size: 1.5em;\n    background-color: rgba(200, 200, 200, 0.9);\n    padding: 0.2em;\n    padding-left: 0.6em;\n    padding-top: 0.5em;\n}\n\n#main #util {\n    background-color: rgba(200, 200, 200, 0.8);\n    padding-bottom: 0.1em;\n}\n\n#main #util .util {\n    margin: 0.4em;\n    margin-left: 1.2em;\n    margin-top: 0.1em;\n    margin-bottom: 1.8em;\n}\n\n#main #util button.util {\n    padding: 0.5em;\n}\n\n#main #util #util-status.status {\n    background-color: rgba(200, 200, 200, 1.0);\n    margin: 0.4em;\n    margin-left: 1.2em;\n    margin-top: 0.1em;\n    margin-bottom: 1.8em;\n    padding: 0.2em;\n    padding-left: 0.6em;\n    padding-top: 0.5em;\n    padding-bottom: 1.2em;\n    font-size: 1.1em;\n    font-weight: 800;\n}\n",""]);const u=s},645:e=>{"use strict";e.exports=function(e){var n=[];return n.toString=function(){return this.map((function(n){var t=e(n);return n[2]?"@media ".concat(n[2]," {").concat(t,"}"):t})).join("")},n.i=function(e,t,o){"string"==typeof e&&(e=[[null,e,""]]);var r={};if(o)for(var i=0;i<this.length;i++){var a=this[i][0];null!=a&&(r[a]=!0)}for(var c=0;c<e.length;c++){var s=[].concat(e[c]);o&&r[s[0]]||(t&&(s[2]?s[2]="".concat(t," and ").concat(s[2]):s[2]=t),n.push(s))}},n}},667:e=>{"use strict";e.exports=function(e,n){return n||(n={}),"string"!=typeof(e=e&&e.__esModule?e.default:e)?e:(/^['"].*['"]$/.test(e)&&(e=e.slice(1,-1)),n.hash&&(e+=n.hash),/["'() \t\n]/.test(e)||n.needQuotes?'"'.concat(e.replace(/"/g,'\\"').replace(/\n/g,"\\n"),'"'):e)}},377:(e,n,t)=>{"use strict";t.r(n),t.d(n,{default:()=>o});const o=t.p+"404.html"},334:(e,n,t)=>{"use strict";t.r(n),t.d(n,{default:()=>o});const o=t.p+"apple-touch-icon-precomposed.png"},820:(e,n,t)=>{"use strict";t.r(n),t.d(n,{default:()=>o});const o=t.p+"favicon.ico"},324:(e,n,t)=>{"use strict";t.r(n),t.d(n,{default:()=>o});const o=t.p+"favicon.png"},166:(e,n,t)=>{"use strict";t.r(n),t.d(n,{default:()=>o});const o=t.p+"index.html"},545:(e,n,t)=>{"use strict";t.d(n,{Z:()=>o});const o=t.p+"world-soup-remastered-bw.png"},530:(e,n,t)=>{"use strict";t.r(n),t.d(n,{default:()=>a});var o=t(379),r=t.n(o),i=t(314);r()(i.Z,{insert:"head",singleton:!1});const a=i.Z.locals||{}},379:(e,n,t)=>{"use strict";var o,r=function(){var e={};return function(n){if(void 0===e[n]){var t=document.querySelector(n);if(window.HTMLIFrameElement&&t instanceof window.HTMLIFrameElement)try{t=t.contentDocument.head}catch(e){t=null}e[n]=t}return e[n]}}(),i=[];function a(e){for(var n=-1,t=0;t<i.length;t++)if(i[t].identifier===e){n=t;break}return n}function c(e,n){for(var t={},o=[],r=0;r<e.length;r++){var c=e[r],s=n.base?c[0]+n.base:c[0],l=t[s]||0,u="".concat(s," ").concat(l);t[s]=l+1;var d=a(u),f={css:c[1],media:c[2],sourceMap:c[3]};-1!==d?(i[d].references++,i[d].updater(f)):i.push({identifier:u,updater:g(f,n),references:1}),o.push(u)}return o}function s(e){var n=document.createElement("style"),o=e.attributes||{};if(void 0===o.nonce){var i=t.nc;i&&(o.nonce=i)}if(Object.keys(o).forEach((function(e){n.setAttribute(e,o[e])})),"function"==typeof e.insert)e.insert(n);else{var a=r(e.insert||"head");if(!a)throw new Error("Couldn't find a style target. This probably means that the value for the 'insert' parameter is invalid.");a.appendChild(n)}return n}var l,u=(l=[],function(e,n){return l[e]=n,l.filter(Boolean).join("\n")});function d(e,n,t,o){var r=t?"":o.media?"@media ".concat(o.media," {").concat(o.css,"}"):o.css;if(e.styleSheet)e.styleSheet.cssText=u(n,r);else{var i=document.createTextNode(r),a=e.childNodes;a[n]&&e.removeChild(a[n]),a.length?e.insertBefore(i,a[n]):e.appendChild(i)}}function f(e,n,t){var o=t.css,r=t.media,i=t.sourceMap;if(r?e.setAttribute("media",r):e.removeAttribute("media"),i&&"undefined"!=typeof btoa&&(o+="\n/*# sourceMappingURL=data:application/json;base64,".concat(btoa(unescape(encodeURIComponent(JSON.stringify(i))))," */")),e.styleSheet)e.styleSheet.cssText=o;else{for(;e.firstChild;)e.removeChild(e.firstChild);e.appendChild(document.createTextNode(o))}}var m=null,p=0;function g(e,n){var t,o,r;if(n.singleton){var i=p++;t=m||(m=s(n)),o=d.bind(null,t,i,!1),r=d.bind(null,t,i,!0)}else t=s(n),o=f.bind(null,t,n),r=function(){!function(e){if(null===e.parentNode)return!1;e.parentNode.removeChild(e)}(t)};return o(e),function(n){if(n){if(n.css===e.css&&n.media===e.media&&n.sourceMap===e.sourceMap)return;o(e=n)}else r()}}e.exports=function(e,n){(n=n||{}).singleton||"boolean"==typeof n.singleton||(n.singleton=(void 0===o&&(o=Boolean(window&&document&&document.all&&!window.atob)),o));var t=c(e=e||[],n);return function(e){if(e=e||[],"[object Array]"===Object.prototype.toString.call(e)){for(var o=0;o<t.length;o++){var r=a(t[o]);i[r].references--}for(var s=c(e,n),l=0;l<t.length;l++){var u=a(t[l]);0===i[u].references&&(i[u].updater(),i.splice(u,1))}t=s}}}}},n={};function t(o){var r=n[o];if(void 0!==r)return r.exports;var i=n[o]={id:o,exports:{}};return e[o](i,i.exports,t),i.exports}t.n=e=>{var n=e&&e.__esModule?()=>e.default:()=>e;return t.d(n,{a:n}),n},t.d=(e,n)=>{for(var o in n)t.o(n,o)&&!t.o(e,o)&&Object.defineProperty(e,o,{enumerable:!0,get:n[o]})},t.g=function(){if("object"==typeof globalThis)return globalThis;try{return this||new Function("return this")()}catch(e){if("object"==typeof window)return window}}(),t.o=(e,n)=>Object.prototype.hasOwnProperty.call(e,n),t.r=e=>{"undefined"!=typeof Symbol&&Symbol.toStringTag&&Object.defineProperty(e,Symbol.toStringTag,{value:"Module"}),Object.defineProperty(e,"__esModule",{value:!0})},(()=>{var e;t.g.importScripts&&(e=t.g.location+"");var n=t.g.document;if(!e&&n&&(n.currentScript&&(e=n.currentScript.src),!e)){var o=n.getElementsByTagName("script");o.length&&(e=o[o.length-1].src)}if(!e)throw new Error("Automatic publicPath is not supported in this browser");e=e.replace(/#.*$/,"").replace(/\?.*$/,"").replace(/\/[^\/]+$/,"/"),t.p=e})(),t(820),t(324),t(334),t(530),t(10),t(377),t(166)})();
/**
 * @type {import('next').NextConfig}
 */
const { PHASE_DEVELOPMENT_SERVER } = require('next/constants');
const path = require('path');

module.exports = (phase, { defaultConfig }) => {
  if (phase === PHASE_DEVELOPMENT_SERVER) {
    return {
      /* development only config options here */
      reactStrictMode: true,
      webpack(config, { dev, isServer }) {
        // why did you render
        if (dev && !isServer) {
          const originalEntry = config.entry;
          config.entry = async () => {
            const wdrPath = path.resolve(__dirname, './scripts/whyDidYouRender.js')
            const entries = await originalEntry();
            if (entries['main.js'] && !entries['main.js'].includes(wdrPath)) {
              entries['main.js'].unshift(wdrPath);
            }
            return entries;
          };
        }

        return config;
      },

    }
  }

  return {
    /* config options for all phases except development here */
    eslint: {
      // Warning: This allows production builds to successfully complete even if
      // your project has ESLint errors.
      ignoreDuringBuilds: true,
    },
    typescript: {
      // !! WARN !!
      // Dangerously allow production builds to successfully complete even if
      // your project has type errors.
      // !! WARN !!
      ignoreBuildErrors: true,
    },
    images:{
      unoptimized: true,
    },
    staticPageGenerationTimeout: 3000,
  }

}

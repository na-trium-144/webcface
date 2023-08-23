import { Client } from "../src/index.js";

const c = new Client("example_recv");
c.logger.info("this is info");
c.logger.warn("this is warn");
c.logger.error("this is error");
setInterval(() => {
  c.sync();
}, 250);

import { assert } from "chai";
import { Client } from "../src/client.js";

describe("Client tests", function () {
  it("should successfully connect within 100ms", function (done) {
    const cli = new Client("");
    setTimeout(function () {
      assert.isOk(cli.connected);
      cli.close();
      done();
    }, 100);
  });
  describe("after sync", function () {
    let cli1: Client, cli2: Client;
    let calledCount = 0;
    beforeEach(function (done) {
      cli1 = new Client("test1");
      cli2 = new Client("test2");
      cli1.value("a").set(1);
      cli1.text("a").set("aaa");
      cli1.func("a").set(() => {
        calledCount++;
      });
      setTimeout(function () {
        cli1.sync();
        cli2.sync();
        setTimeout(done, 100);
      }, 100);
    });
    afterEach(function () {
      cli1.close();
      cli2.close();
    });
    describe("Member", function () {
      it("should receive other member's name after sync", function () {
        assert.lengthOf(cli1.members(), 1);
        assert.strictEqual(cli1.members()[0].name, cli2.name);
      });
    });
  });
});

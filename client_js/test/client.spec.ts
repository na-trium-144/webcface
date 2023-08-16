import { assert } from "chai";
import { Client } from "../src/client.js";
import { Member } from "../src/data.js";
import { argType } from "../src/message.js";

describe("Client tests", function () {
  it("should successfully connect within 100ms", function (done) {
    const cli = new Client("");
    setTimeout(function () {
      assert.isOk(cli.connected);
      cli.close();
      done();
    }, 100);
  });
  describe("before sync", function () {
    let cli1: Client, cli2: Client;
    beforeEach(function (done) {
      cli1 = new Client("test1");
      cli2 = new Client("test2");
      setTimeout(done, 100);
    });
    afterEach(function () {
      cli1.close();
      cli2.close();
    });
    it("should have no members", function () {
      assert.isEmpty(cli1.members());
    });
  });
  describe("after sync", function () {
    let cli1: Client, cli2: Client;
    let m1: Member;
    beforeEach(function (done) {
      cli1 = new Client("test1");
      cli2 = new Client("test2");
      cli2.value("a").set(1);
      cli2.text("a").set("aaa");
      cli2.func("a").set(() => undefined, argType.number_, [
        { name: "a", type: argType.int_, init: 3, min: 2, max: 7 },
        { name: "a", type: argType.string_, option: ["a", "b"] },
      ]);
      m1 = cli1.member(cli2.name);
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
    it("should receive member name", function () {
      assert.lengthOf(cli1.members(), 1);
      assert.strictEqual(cli1.members()[0].name, cli2.name);
    });
    it("should receive value name", function () {
      assert.lengthOf(m1.values(), 1);
      assert.strictEqual(m1.values()[0].name, "a");
    });
    it("should receive text name", function () {
      assert.lengthOf(m1.texts(), 1);
      assert.strictEqual(m1.texts()[0].name, "a");
    });
    it("should receive func name", function () {
      assert.lengthOf(m1.funcs(), 1);
      assert.strictEqual(m1.funcs()[0].name, "a");
    });
    it("should receive func information", function () {
      assert.strictEqual(m1.func("a").returnType, cli2.func("a").returnType);
      assert.deepEqual(m1.func("a").args, [
        { name: "a", type: argType.int_, init: 3, min: 2, max: 7, option: [] },
        {
          name: "a",
          type: argType.string_,
          option: ["a", "b"],
          init: null,
          min: null,
          max: null,
        },
      ]);
    });
  });
});

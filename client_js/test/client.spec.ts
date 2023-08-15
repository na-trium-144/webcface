import { assert } from "chai";
import { Client } from "../src/client.js";
import { Member } from "../src/data.js";

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
    describe("member", function () {
      it("should have no members", function () {
        assert.lengthOf(cli1.members(), 0);
      });
    });
  });
  describe("after sync", function () {
    let cli1: Client, cli2: Client;
    let m1: Member;
    let calledCount = 0;
    beforeEach(function (done) {
      cli1 = new Client("test1");
      cli2 = new Client("test2");
      cli2.value("a").set(1);
      cli2.text("a").set("aaa");
      cli2.func("a").set(() => {
        calledCount++;
      });
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
    describe("member", function () {
      it("should receive name", function () {
        assert.lengthOf(cli1.members(), 1);
        assert.strictEqual(cli1.members()[0].name, cli2.name);
      });
    });
    describe("value", function () {
      it("should receive name", function () {
        assert.lengthOf(m1.values(), 1);
        assert.strictEqual(m1.values()[0].name, "a");
      });
      it("should be null before sync", function () {
        assert.isNull(m1.value("a").tryGet());
      });
      it("should have value after sync", function (done) {
        m1.value("a").tryGet();
        cli1.sync();
        setTimeout(function () {
          assert.strictEqual(m1.value("a").tryGet(), cli2.value("a").get());
          done();
        }, 100);
      });
    });
    describe("text", function () {
      it("should receive name", function () {
        assert.lengthOf(m1.texts(), 1);
        assert.strictEqual(m1.texts()[0].name, "a");
      });
      it("should be null before sync", function () {
        assert.isNull(m1.text("a").tryGet());
      });
      it("should have value after sync", function (done) {
        m1.text("a").tryGet();
        cli1.sync();
        setTimeout(function () {
          assert.strictEqual(m1.text("a").tryGet(), cli2.text("a").get());
          done();
        }, 100);
      });
    });
  });
});

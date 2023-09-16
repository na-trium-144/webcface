import { assert } from "chai";
import { ClientData, SyncDataStore2 } from "../src/clientData.js";

describe("ClientData Tests", function () {
  const selfName = "test";
  describe("SyncDataStore2", function () {
    let s2: SyncDataStore2<string>;
    beforeEach(function () {
      s2 = new SyncDataStore2<string>(selfName);
    });
    describe("#isSelf()", function () {
      it("returns true when self name is passed", function () {
        assert.isTrue(s2.isSelf(selfName));
      });
      it("returns false when different name is passed", function () {
        assert.isFalse(s2.isSelf("hoge"));
      });
      it("returns false when empty string is passed", function () {
        assert.isFalse(s2.isSelf(""));
      });
    });
    describe("#setSend()", function () {
      it("sets value to #dataSend", function () {
        s2.setSend("a", "b");
        assert.strictEqual(s2.dataSend.get("a"), "b");
      });
      it("sets value to #dataRecv", function () {
        s2.setSend("a", "b");
        assert.strictEqual(s2.dataRecv.get(selfName)?.get("a"), "b");
      });
    });
    describe("#setHidden()", function () {
      it("sets value to #dataSendHidden", function () {
        s2.setHidden("a", true);
        assert.isTrue(s2.dataSendHidden.get("a"));
      });
    });
    describe("#isHidden()", function () {
      it("returns false when it is set false", function () {
        s2.dataSendHidden.set("a", false);
        assert.isFalse(s2.isHidden("a"));
      });
      it("returns true when it is set true", function () {
        s2.dataSendHidden.set("a", true);
        assert.isTrue(s2.isHidden("a"));
      });
      it("returns false bt default", function () {
        assert.isFalse(s2.isHidden("a"));
      });
    });
    describe("#setRecv()", function () {
      it("sets value to #dataRecv", function () {
        s2.setRecv("a", "b", "c");
        assert.strictEqual(s2.dataRecv.get("a")?.get("b"), "c");
      });
    });
    describe("#getRecv()", function () {
      it("returns value in #dataRecv", function () {
        s2.dataRecv.set("a", new Map([["b", "c"]]));
        assert.strictEqual(s2.getRecv("a", "b"), "c");
      });
      it("sets new request to #req and #reqSend", function () {
        s2.getRecv("a", "b");
        assert.strictEqual(s2.req.get("a")?.get("b"), 1);
        assert.strictEqual(s2.reqSend.get("a")?.get("b"), 1);
      });
      it("does not set #req and #reqSend if member is self name", function () {
        s2.getRecv(selfName, "b");
        assert.isEmpty(s2.req);
        assert.isEmpty(s2.reqSend);
      });
    });
    describe("#unsetRecv()", function () {
      it("removes value from #dataRecv", function () {
        s2.dataRecv.set("a", new Map([["b", "c"]]));
        s2.unsetRecv("a", "b");
        assert.isFalse(s2.dataRecv.get("a")?.has("b"));
      });
      it("sets 0 to #req and #reqSend if already set", function () {
        s2.dataRecv.set("a", new Map([["b", "c"]]));
        s2.req.set("a", new Map([["b", 1]]));
        s2.reqSend.set("a", new Map([["b", 1]]));
        s2.unsetRecv("a", "b");
        assert.strictEqual(s2.req.get("a")?.get("b"), 0);
        assert.strictEqual(s2.reqSend.get("a")?.get("b"), 0);
      });
    });
    describe("#addMember()", function () {
      it("add member to #entry", function () {
        s2.addMember("a");
        assert.isTrue(s2.entry.has("a"));
        assert.isEmpty(s2.entry.get("a"));
      });
    });
    describe("#addEntry()", function () {
      it("add entry to #entry", function () {
        s2.setEntry("a", "b");
        assert.sameOrderedMembers(s2.entry.get("a") || [], ["b"]);
      });
    });
    describe("#getMembers()", function () {
      it("returns members in #entry", function () {
        s2.entry.set("a", []);
        s2.entry.set("b", []);
        assert.sameMembers(s2.getMembers(), ["a", "b"]);
      });
    });
    describe("#getEntry()", function () {
      it("returns entries in #entry", function () {
        s2.entry.set("a", ["a", "b", "c"]);
        assert.sameMembers(s2.getEntry("a"), ["a", "b", "c"]);
      });
    });
    describe("#transferSend()", function () {
      beforeEach(function () {
        s2.dataSend.set("a", "a");
        s2.dataRecv.set(
          selfName,
          new Map([
            ["a", "a"],
            ["b", "b"],
          ])
        );
      });
      it("returns #dataSend when isFirst = false", function () {
        const s = s2.transferSend(false);
        assert.strictEqual(s.get("a"), "a");
        assert.isFalse(s.has("b"));
      });
      it("returns nothing on second call with isFirst = false", function () {
        s2.transferSend(false);
        const s = s2.transferSend(false);
        assert.isEmpty(s);
      });
      it("returns all values with self name in #dataRecv when isFirst = true", function () {
        const s = s2.transferSend(true);
        assert.strictEqual(s.get("a"), "a");
        assert.strictEqual(s.get("b"), "b");
      });
    });
    describe("#getSendPrev()", function () {
      beforeEach(function () {
        s2.dataSend.set("a", "a");
        s2.dataRecv.set(
          selfName,
          new Map([
            ["a", "a"],
            ["b", "b"],
          ])
        );
      });
      it("returns previous #dataSend when isFirst = false", function () {
        s2.transferSend(false);
        const s = s2.getSendPrev(false);
        assert.strictEqual(s.get("a"), "a");
        assert.isFalse(s.has("b"));
      });
      it("returns nothing when isFirst = true", function () {
        const s = s2.getSendPrev(true);
        assert.isEmpty(s);
      });
    });
    describe("#transferReq", function () {
      beforeEach(function () {
        s2.reqSend.set("a", new Map([["b", 1]]));
        s2.req.set(
          "a",
          new Map([
            ["b", 1],
            ["c", 2],
          ])
        );
      });
      it("returns #reqSend when isFirst = false", function () {
        const s = s2.transferReq(false);
        assert.strictEqual(s.get("a")?.get("b"), 1);
        assert.isFalse(s.get("a")?.has("c"));
      });
      it("returns nothing on second call with isFirst = false", function () {
        s2.transferReq(false);
        const s = s2.transferReq(false);
        assert.isEmpty(s);
      });
      it("returns #req when isFirst = true", function () {
        const s = s2.transferReq(true);
        assert.strictEqual(s.get("a")?.get("b"), 1);
        assert.strictEqual(s.get("a")?.get("c"), 2);
      });
    });
    describe("#getReq()", function () {
      beforeEach(function () {
        s2.req.set(
          "a",
          new Map([
            ["b", 1],
            ["c", 2],
          ])
        );
      });
      it("returns member and field of given req_id", function () {
        assert.sameOrderedMembers(s2.getReq(1), ["a", "b"]);
      });
      it("returns empty member and field when req_id not found", function () {
        assert.sameOrderedMembers(s2.getReq(999), ["", ""]);
      });
    });
  });
});

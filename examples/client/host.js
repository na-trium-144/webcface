async function updateHostInfo() {
  const host = await (await fetch("/_hostinfo")).json();
  document.title = `${host.name} - WebCFace`;
  document.getElementById('header').innerText = `WebCFace @ ${host.name} (${host.addr})`;
}
updateHostInfo();

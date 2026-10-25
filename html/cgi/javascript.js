["py", "php"].forEach((cgi) => {
  const ENDPOINT = `${cgi}/cgi.${cgi}`;

  /** @type {HTMLFormElement} */
  const $form = document.querySelector(`#${cgi}`);

  /** @type {HTMLInputElement} */
  const $input = $form.querySelector("input[type='text']");

  /** @type {HTMLUListElement} */
  const $ul = $form.querySelector("ul");

  /** @type {string[]} */
  let names = [];

  const handleRefresh = async () => {
    try {
      const response = await fetch(ENDPOINT);

      if (!response.ok) {
        throw new Error("Failed to fetch names");
      }

      names = await response.json();
    } catch {
      names = [];
      alert(`Failed to fetch for ${cgi}`);
    }

    $ul.innerHTML = "";

    if (names.length === 0) {
      const $li = document.createElement("li");
      $li.style.fontWeight = "bold";
      $li.textContent = "NIL";
      $ul.appendChild($li);
      return;
    }

    names.forEach((name) => {
      const $li = document.createElement("li");
      const $del_button = document.createElement("button");
      const $text = document.createElement("span");

      $text.textContent = name;

      $del_button.textContent = "x";
      $del_button.style.marginLeft = "0.5rem";

      $li.appendChild($text);
      $li.appendChild($del_button);
      $ul.appendChild($li);

      $del_button.addEventListener("click", (e) => {
        e.preventDefault();
        e.stopPropagation();
        handleDelete(name);
      });
    });
  };

  const handleDelete = async (name) => {
    try {
      const response = await fetch(
        `${ENDPOINT}?name=${encodeURIComponent(name)}`,
        { method: "DELETE" },
      );

      if (!response.ok) {
        throw new Error("Failed to delete name");
      }

      await handleRefresh();
    } catch {
      alert(`Failed to delete for ${cgi}`);
    }
  };

  const handleCreate = async (name) => {
    if (names.includes(name)) {
      alert("Cannot add duplicate");
      return;
    }

    try {
      const response = await fetch(ENDPOINT, {
        method: "POST",
        body: name,
      });

      if (!response.ok) {
        throw new Error("Failed to add name");
      }

      await handleRefresh();
      $input.value = "";
    } catch {
      alert(`Failed to add for ${cgi}`);
    }
  };

  $form.addEventListener("submit", (e) => {
    e.preventDefault();
    handleCreate($input.value);
  });

  handleRefresh();
});

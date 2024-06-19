// A mockup for NodeJS fs module

const readFileSync = {};
const fs = {
    readFileSync: readFileSync,
};
export default fs;
export { readFileSync };

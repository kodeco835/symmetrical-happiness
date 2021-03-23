import { css } from '@emotion/core';
import { palette } from '@expo/styleguide';

import { paragraph } from '~/components/base/typography';
import * as Constants from '~/constants/theme';

export const globalTippy = css`
  div.tippy-tooltip {
    text-align: left;
    background: ${palette.dark.black};
  }

  .tippy-popper[x-placement^='top'] .tippy-tooltip .tippy-roundarrow {
    fill: ${palette.dark.black};
  }

  .tippy-tooltip.expo-theme .tippy-content {
    ${paragraph};
    color: ${palette.dark.gray[900]};
    font-family: ${Constants.fonts.book};
    background: ${palette.dark.black};
    padding: 8px;
  }

  .tippy-content a {
    color: ${palette.dark.gray[900]};
  }
`;
